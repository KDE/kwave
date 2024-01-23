/***************************************************************************
 PlaybackController.cpp  -  Interface for generic playback control
                             -------------------
    begin                : Nov 15 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"

#include <math.h>
#include <new>

#include <QMutexLocker>

#include "libkwave/MessageBox.h"
#include "libkwave/MixerMatrix.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/PlayBackDevice.h"
#include "libkwave/PlayBackTypesMap.h"
#include "libkwave/PlaybackController.h"
#include "libkwave/PlaybackDeviceFactory.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

/** Sets the number of screen refreshes per second when in playback mode */
#define SCREEN_REFRESHES_PER_SECOND 10

//***************************************************************************
Kwave::PlaybackController::PlaybackController(
    Kwave::SignalManager &signal_manager
)
    :m_signal_manager(signal_manager), m_thread(this, QVariant()),
     m_device(Q_NULLPTR), m_lock_device(), m_playback_params(),
     m_lock_playback(), m_should_seek(false), m_seek_pos(0),
     m_track_selection_changed(false),
     m_reload_mode(false), m_loop_mode(false), m_paused(false),
     m_playing(false), m_playback_position(0), m_playback_start(0),
     m_playback_end(0), m_old_first(0), m_old_last(0),
     m_playback_factories()
{

    connect(this, SIGNAL(sigDevicePlaybackDone()),
            this, SLOT(playbackDone()));
    connect(this, SIGNAL(sigDevicePlaybackDone()),
            this, SLOT(closeDevice()),
            Qt::QueuedConnection);
    connect(&m_signal_manager, SIGNAL(sigTrackSelectionChanged(bool)),
            this,              SLOT(trackSelectionChanged()));

}

//***************************************************************************
Kwave::PlaybackController::~PlaybackController()
{
    playbackStop();
}

//***************************************************************************
void Kwave::PlaybackController::playbackStart()
{
    // leave the reload mode in any case
    m_reload_mode = false;

    if (m_playing) {
        // first stop playback
        stopDevicePlayBack();
        emit sigPlaybackStopped();
    }

    // (re)start from beginning without loop mode
    m_playback_position = m_playback_start;
    emit sigPlaybackPos(m_playback_position);

    m_loop_mode = false;
    m_paused = false;
    m_playing = true;
    emit sigPlaybackStarted();

    startDevicePlayBack();
}

//***************************************************************************
void Kwave::PlaybackController::playbackLoop()
{
    // leave the reload mode in any case
    m_reload_mode = false;

    if (m_playing) {
        // first stop playback
        stopDevicePlayBack();
        m_playing = false;
        emit sigPlaybackStopped();
    }

    // (re)start from beginning without loop mode
    m_playback_position = m_playback_start;
    emit sigPlaybackPos(m_playback_position);

    m_loop_mode = true;
    m_paused = false;
    startDevicePlayBack();

    m_playing = true;
    emit sigPlaybackStarted();
}

//***************************************************************************
void Kwave::PlaybackController::playbackPause()
{
    // leave the reload mode in any case
    m_reload_mode = false;

    if (!m_playing) return; // no effect if not playing

    m_paused = true;

    // stop playback for now and set the paused flag
    stopDevicePlayBack();
}

//***************************************************************************
void Kwave::PlaybackController::playbackContinue()
{
    // leave the reload mode in any case
    m_reload_mode = false;

    // if not paused, do the same as start
    if (!m_paused) {
        playbackStart();
        return;
    }

    // else reset the paused flag and start from current position
    startDevicePlayBack();

    m_paused = false;
    m_playing = true;

    emit sigPlaybackStarted();
}

//***************************************************************************
void Kwave::PlaybackController::playbackStop()
{
    // leave the reload mode in any case
    m_reload_mode = false;

    // stopped in pause state
    if (m_paused) {
        m_playing = false;
        m_paused = false;
        emit sigPlaybackStopped();
    }
    if (!m_playing) return; // already stopped
    stopDevicePlayBack();
}

//***************************************************************************
void Kwave::PlaybackController::seekTo(sample_index_t pos)
{
    if (pos < m_playback_start) pos = m_playback_start;
    if (pos > m_playback_end)   pos = m_playback_end;

    {
        QMutexLocker lock(&m_lock_playback);
        qDebug("seekTo(%llu)", pos);
        m_seek_pos    = pos;
        m_should_seek = true;
    }

    if (m_paused) {
        // if playback is paused, we want an update of the playback
        // position anyway. as this will not come from the device layer,
        // fake an update right here
        updatePlaybackPos(pos);
        seekDone(pos);
    }
}

//***************************************************************************
void Kwave::PlaybackController::seekDone(sample_index_t pos)
{
    emit sigSeekDone(pos);
}

//***************************************************************************
void Kwave::PlaybackController::updatePlaybackPos(sample_index_t pos)
{
    m_playback_position = pos;
    emit sigPlaybackPos(m_playback_position); // TODO => per TIMER !!!
}

//***************************************************************************
void Kwave::PlaybackController::playbackDone()
{
    if (m_reload_mode) {
        // if we were in the reload mode, reset the
        // paused flag and start again from current position
        startDevicePlayBack();
        m_paused = false;
        m_playing = true;

        // leave the "reload" mode
        m_reload_mode = false;
        return;
    }

    m_playing = false;
    if (m_paused)
        emit sigPlaybackPaused();
    else {
        emit sigPlaybackPos(m_playback_position);
        emit sigPlaybackStopped();
    }

    m_old_first = 0;
    m_old_last = 0;
}

//***************************************************************************
void Kwave::PlaybackController::reload()
{
    if (!m_playing || m_paused) return; // no effect if not playing or paused

    // enter the "reload" mode
    m_reload_mode = true;

    // stop playback for now and set the paused flag
    m_paused = true;
    stopDevicePlayBack();
}

//***************************************************************************
void Kwave::PlaybackController::reset()
{
    m_playback_start = 0;
    m_playback_position = 0;
    m_loop_mode = false;
    m_playing = false;
    m_paused = false;
    m_reload_mode = false;

    emit sigPlaybackPos(0);
    emit sigPlaybackStopped();
}

//***************************************************************************
bool Kwave::PlaybackController::loop() const
{
    return m_loop_mode;
}

//***************************************************************************
bool Kwave::PlaybackController::running() const
{
    return m_playing;
}

//***************************************************************************
bool Kwave::PlaybackController::paused() const
{
    return m_paused;
}

//***************************************************************************
void Kwave::PlaybackController::setStartPos(sample_index_t pos)
{
    m_playback_start = pos;
}

//***************************************************************************
void Kwave::PlaybackController::setEndPos(sample_index_t pos)
{
    m_playback_end = pos;
}

//***************************************************************************
sample_index_t Kwave::PlaybackController::startPos() const
{
    return m_playback_start;
}

//***************************************************************************
sample_index_t Kwave::PlaybackController::endPos() const
{
    return m_playback_end;
}

//***************************************************************************
sample_index_t Kwave::PlaybackController::currentPos() const
{
    return m_playback_position;
}

//***************************************************************************
void Kwave::PlaybackController::startDevicePlayBack()
{
    // set the real sample rate for playback from the signal itself
    m_playback_params.rate = m_signal_manager.rate();

    QMutexLocker lock_for_delete(&m_lock_device);

    // remove the old device if still one exists
    if (m_device) {
        qWarning("PlaybackController::startDevicePlayBack(): "
                 "removing stale instance");
        delete m_device;
        m_device = Q_NULLPTR;
    }

    // open the device and abort if not possible

//     qDebug("PlaybackController::startDevicePlayBack(), device='%s'",
//           DBG(m_playback_params.device));
    m_device = openDevice(-1, &m_playback_params);
    if (!m_device) {
        // simulate a "playback done" on errors
        emit sigDevicePlaybackDone();
        return;
    }

    sample_index_t first = m_signal_manager.selection().first();
    sample_index_t last  = m_signal_manager.selection().last();

    if (m_paused) {
        // continue after pause
        if ((m_old_first != first) || (m_old_last != last)) {
            // selection has changed
            if (first != last) {
                // something selected -> set new range
                setStartPos(first);
                setEndPos(last);

                sample_index_t pos = currentPos();
                if ((pos < first) || (pos > last)) {
                    // completely new area selected, or the right margin
                    // has been moved before the current playback pointer
                    // -> play from start of new selection
                    updatePlaybackPos(first);
                }
            } else {
                // nothing selected -> select all and move to position
                setStartPos(first);
                setEndPos(m_signal_manager.length() - 1);
            }
        }
    } else {
        // determine first and last sample if not in paused mode"
        if (first == last) {
            // nothing selected -> play from cursor position
            setStartPos(first);
            setEndPos(m_signal_manager.length() - 1);
        } else {
            // play only in selection
            setStartPos(first);
            setEndPos(last);
        }
        updatePlaybackPos(first);
    }

    m_old_first = first;
    m_old_last = last;

    m_thread.start();
}

//***************************************************************************
void Kwave::PlaybackController::stopDevicePlayBack()
{
    m_thread.requestInterruption();
    if (!m_thread.isRunning()) {
        qDebug("PlaybackController::stopDevicePlayBack() - not running");
        emit sigDevicePlaybackDone();
    }
    closeDevice();
}

//***************************************************************************
void Kwave::PlaybackController::trackSelectionChanged()
{
    QMutexLocker lock(&m_lock_playback);
    m_track_selection_changed = true;
}

//***************************************************************************
void Kwave::PlaybackController::run_wrapper(const QVariant &params)
{
    Q_UNUSED(params)

    Kwave::MixerMatrix *mixer = Q_NULLPTR;
    sample_index_t first      = m_playback_start;
    sample_index_t last       = m_playback_end;
    unsigned int out_channels = m_playback_params.channels;

    QVector<unsigned int> all_tracks = m_signal_manager.allTracks();
    unsigned int tracks = all_tracks.count();
    QVector<unsigned int> audible_tracks = m_signal_manager.selectedTracks();
    unsigned int audible_count = audible_tracks.count();

    // get the list of selected channels
    if (!tracks || !m_device) {
        // not even one selected track or no (open) device
        qDebug("PlaybackController::run(): no audible track(s) !");
        emit sigDevicePlaybackDone();
        return;
    }

    // set up a set of sample reader (streams)
    Kwave::MultiTrackReader input(
        Kwave::FullSnapshot,
        m_signal_manager, all_tracks, first, last);

    // create a new translation matrix for mixing up/down to the desired
    // number of output channels
    m_track_selection_changed = false;

    // loop until process is stopped
    // or run once if not in loop mode
    Kwave::SampleArray in_samples(tracks);
    Kwave::SampleArray out_samples(out_channels);
    sample_index_t pos = m_playback_position;
    updatePlaybackPos(pos);

    // counter for refresh of the playback position
    unsigned int pos_countdown = 0;

    do {

        // if current position is after start -> skip the passed
        // samples (this happens when resuming after a pause)
        if (pos > first) input.skip(pos - first);

        while ((pos++ <= last) && !m_thread.isInterruptionRequested()) {
            unsigned int x;
            unsigned int y;
            bool seek_again = false;
            bool seek_done  = false;

            {
                QMutexLocker _lock(&m_lock_playback);

                // check for track selection change (need for new mixer)
                if (m_track_selection_changed) {
                    if (mixer) delete mixer;
                    mixer = Q_NULLPTR;
                    m_track_selection_changed = false;
                }

                if (!mixer) {
                    audible_tracks = m_signal_manager.selectedTracks();
                    audible_count = audible_tracks.count();
                    mixer = new(std::nothrow)
                        Kwave::MixerMatrix(audible_count, out_channels);
                    Q_ASSERT(mixer);
                    if (!mixer) break;
                    seek_again = true; // re-synchronize all reader positions
                }

                // check for seek requests
                if (m_should_seek && (m_seek_pos != pos)) {
                    if (m_seek_pos < first) m_seek_pos = first;
                    if (m_seek_pos > last)  { pos = last; break; }
                    pos = m_seek_pos;
                    m_should_seek = false;
                    seek_again = true;
                    seek_done  = true;
                }
            }

            if (seek_again) input.seek(pos);
            if (seek_done)  seekDone(pos);

            // fill input buffer with samples
            for (x = 0; x < audible_count; ++x) {
                in_samples[x] = 0;
                Kwave::SampleReader *stream = input[audible_tracks[x]];
                Q_ASSERT(stream);
                if (!stream) continue;

                if (!stream->eof()) (*stream) >> in_samples[x];
            }

            // multiply matrix with input to get output
            const Kwave::SampleArray &in = in_samples;
            for (y = 0; y < out_channels; ++y) {
                double sum = 0;
                for (x = 0; x < audible_count; ++x) {
                    sum += static_cast<double>(in[x]) * (*mixer)[x][y];
                }
                out_samples[y] = static_cast<sample_t>(sum);
            }

            // write samples to the playback device
            int result = -1;
            {
                unsigned int retry = 10;
                while (retry-- && !m_thread.isInterruptionRequested()) {
                    QMutexLocker lock(&m_lock_device);
                    if (m_device)
                        result = m_device->write(out_samples);
                    if (result == 0)
                        break;
                }
            }
            if (result) {
                m_thread.requestInterruption();
                pos = last;
            }

            // update the playback position if timer elapsed
            if (!pos_countdown) {
                pos_countdown = Kwave::toUint(ceil(
                    m_playback_params.rate / SCREEN_REFRESHES_PER_SECOND));
                updatePlaybackPos(pos);
            } else {
                --pos_countdown;
            }
        }

        // maybe we loop. in this case the playback starts
        // again from the left marker
        if (m_loop_mode && !m_thread.isInterruptionRequested()) {
            input.reset();
            pos = startPos();
        }

    } while (m_loop_mode && !m_thread.isInterruptionRequested());

    // playback is done
    emit sigDevicePlaybackDone();
//     qDebug("PlaybackController::run() done.");
}

//***************************************************************************
void Kwave::PlaybackController::closeDevice()
{
    Kwave::PlayBackDevice *dev = Q_NULLPTR;
    if (m_device) {
        // NOTE: we could get a recursion here if we delete with the lock
        //       held, if the device calls processEvents during shutdown
        QMutexLocker lock_for_delete(&m_lock_device);
        dev = m_device;
        m_device = Q_NULLPTR;
    }
    delete dev;
}

//***************************************************************************
void Kwave::PlaybackController::checkMethod(Kwave::playback_method_t &method)
{
    QList<Kwave::playback_method_t> all_methods;

    // create a list of all supported playback methods
    foreach (Kwave::PlaybackDeviceFactory *f, m_playback_factories) {
        QList<Kwave::playback_method_t> methods = f->supportedMethods();

        // return immediately on a direct match
        if (methods.contains(method)) return;

        // otherwise accumulate all found methods
        foreach (Kwave::playback_method_t m, methods)
            if (!all_methods.contains(m))
                all_methods.append(m);
    }

    // no direct match found: take the best match (lowest number)
    Kwave::playback_method_t best = Kwave::PLAYBACK_INVALID;
    foreach (Kwave::playback_method_t m, all_methods) {
        if (m == Kwave::PLAYBACK_NONE) continue; // not a valid selection
        if (m < best) best = m;
    }

    Kwave::PlayBackTypesMap map;
    qDebug("playback method '%s' (%d) not supported "
           "-> falling back to '%s' (%d)",
           DBG(map.name(map.findFromData(method))), static_cast<int>(method),
           DBG(map.name(map.findFromData(best))),   static_cast<int>(best)
    );

    method = best;
}

//***************************************************************************
Kwave::PlayBackDevice *Kwave::PlaybackController::createDevice(
    Kwave::playback_method_t method)
{
    // locate the corresponding playback device factory (plugin)
    Kwave::PlaybackDeviceFactory *factory = Q_NULLPTR;
    foreach (Kwave::PlaybackDeviceFactory *f, m_playback_factories) {
        Q_ASSERT(f);
        if (f && f->supportedMethods().contains(method)) {
            factory = f;
            break;
        }
    }
    if (!factory) return Q_NULLPTR;

    // create a new device instance, using the given method
    return factory->createDevice(method);
}

//***************************************************************************
Kwave::PlayBackDevice *Kwave::PlaybackController::openDevice(
    int tracks,
    const Kwave::PlayBackParam *playback_params)
{
    // take playback parameters if given,
    // otherwise fall back to current defaults
    Kwave::PlayBackParam params = (playback_params) ?
        *playback_params : m_playback_params;

    // if no playback parameters specified or no method selected:
    // -> auto-detect best
    if (!playback_params || (params.method == PLAYBACK_NONE))
        checkMethod(params.method);

    if (!playback_params) {
        if (!m_signal_manager.isClosed() && !m_signal_manager.isEmpty()) {
            params.rate     = m_signal_manager.rate();
            params.channels = m_signal_manager.selectedTracks().count();
        }
    }

    // try to create a new device, using the given playback method
    Kwave::PlayBackDevice *device = createDevice(params.method);
    if (!device) return Q_NULLPTR;

    // override the number of tracks if not negative
    if (tracks > 0) params.channels = tracks;

    // open the playback device with it's default parameters
    // open and initialize the device
    QString result = device->open(
        params.device,
        params.rate,
        params.channels,
        params.bits_per_sample,
        params.bufbase
    );
    if (result.length()) {
        qWarning("PlayBackPlugin::openDevice(): opening the device failed.");

        // delete the device if it did not open
        delete device;
        device = Q_NULLPTR;

        // show an error message box
        Kwave::MessageBox::error(m_signal_manager.parentWidget(), result,
            i18n("Unable to open '%1'",
            params.device.section(QLatin1Char('|'), 0, 0)));
    }


    return device;
}

//***************************************************************************
void Kwave::PlaybackController::setDefaultParams(
    const Kwave::PlayBackParam &params)
{
    m_playback_params = params;
}

//***************************************************************************
void Kwave::PlaybackController::registerPlaybackDeviceFactory(
    Kwave::PlaybackDeviceFactory *factory)
{
    m_playback_factories.append(factory);
}

//***************************************************************************
void Kwave::PlaybackController::unregisterPlaybackDeviceFactory(
    Kwave::PlaybackDeviceFactory *factory)
{
    m_playback_factories.removeAll(factory);
}

//***************************************************************************
//***************************************************************************
