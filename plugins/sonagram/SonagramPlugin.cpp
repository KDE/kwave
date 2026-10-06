/***************************************************************************
              SonagramPlugin.cpp  -  plugin that shows a sonagram window
                             -------------------
    begin                : Fri Jul 28 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
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

#include <errno.h>
#include <math.h>
#include <qnumeric.h>
#include <qtypes.h>
#include <stdlib.h>

#include <complex>
#include <limits>
#include <new>

#include <QApplication>
#include <QColor>
#include <QFutureSynchronizer>
#include <QMutexLocker>
#include <QPointer>
#include <QString>
#include <QtConcurrentRun>

#include "libkwave/GlobalLock.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Plugin.h"
#include "libkwave/PluginManager.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Track.h"
#include "libkwave/Utils.h"
#include "libkwave/WindowFunction.h"

#include "libgui/OverViewCache.h"
#include "libgui/SelectionTracker.h"

#include "SonagramDialog.h"
#include "SonagramPlugin.h"
#include "SonagramWindow.h"

KWAVE_PLUGIN(sonagram, SonagramPlugin)

/**
 * interval for limiting the number of repaints per second [ms]
 */
#define REPAINT_INTERVAL 250

//***************************************************************************
Kwave::SonagramPlugin::SonagramPlugin(QObject *parent,
                                      const QVariantList &args)
    :Kwave::Plugin(parent, args),
     m_mode(MODE_VIEW),
     m_sonagram_window(nullptr),
     m_selection(nullptr),
     m_slices(0), m_fft_points(0),
     m_window_type(Kwave::WINDOW_FUNC_NONE),
     m_color_mode(COLOR_MODE_NONE),
     m_track_changes(true), m_follow_selection(false), m_image(),
     m_overview_cache(nullptr), m_slice_pool(), m_valid(MAX_SLICES, false),
     m_pending_jobs(), m_lock_job_list(), m_future(),
     m_repaint_timer()
{
    i18n("Sonagram");

    // connect the output of the sonagram worker thread
    connect(this, SIGNAL(sliceAvailable(Kwave::SonagramPlugin::Slice*)),
            this, SLOT(insertSlice(Kwave::SonagramPlugin::Slice*)),
            Qt::QueuedConnection);

    // connect repaint timer
    connect(&m_repaint_timer, SIGNAL(timeout()),
            this, SLOT(validate()));
}

//***************************************************************************
Kwave::SonagramPlugin::~SonagramPlugin()
{
    m_repaint_timer.stop();

    delete m_sonagram_window;
    m_sonagram_window = nullptr;

    delete m_selection;
    m_selection = nullptr;
}

//***************************************************************************
QStringList *Kwave::SonagramPlugin::setup(QStringList &previous_params)
{
    QStringList *result = nullptr;

    // try to interpret the list of previous parameters, ignore errors
    if (previous_params.count()) interpreteParameters(previous_params);

    QPointer<Kwave::SonagramDialog> dlg =
        new(std::nothrow) Kwave::SonagramDialog(*this);
    Q_ASSERT(dlg);
    if (!dlg) return nullptr;

    dlg->setWindowFunction(m_window_type);
    dlg->setColorMode(m_color_mode);
    dlg->setTrackChanges(m_track_changes);
    dlg->setFollowSelection(m_follow_selection);

    if ((dlg->exec() == QDialog::Accepted) && dlg) {
        result = new(std::nothrow) QStringList();
        Q_ASSERT(result);
        if (result) dlg->parameters(*result);
    }

    delete dlg;
    return result;
}

//***************************************************************************
int Kwave::SonagramPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    // we have different modes:
    // 1) <fft points>, <window type>, <color>,
    //    <track changes>, <follow_selection>,
    //    ["window"|"view"]
    // 2) "load", <filename>, ["window"|"view"]
    // 3) "save", <filename>

    if ((params.count() >= 2) && (params[0] == _("load"))) {
        // determine the last used window function
        QStringList last_params = manager().defaultParams(name());
        param = last_params[1];
        m_window_type = Kwave::WindowFunction::findFromName(param);

        m_mode = MODE_LOAD;
        m_fft_points       = 0;
        m_color_mode       = COLOR_MODE_NONE;
        m_track_changes    = false;
        m_follow_selection = false;
        return 0;
    } else if ((params.count() >= 3) && (params[0] == _("save"))) {
        // nothing to do here, saving is done in start()
        m_mode = MODE_SAVE;
        return 0;
    } else if (params.count() >= 5) {
        m_mode = MODE_VIEW;

        param = params[0];
        m_fft_points = param.toUInt(&ok);
        if (!ok) return -EINVAL;
        if (m_fft_points > MAX_FFT_POINTS) m_fft_points = MAX_FFT_POINTS;

        param = params[1];
        m_window_type = Kwave::WindowFunction::findFromName(param);

        param = params[2];
        unsigned int mode = param.toUInt(&ok);
        if (!ok) return -EINVAL;
        switch (mode)
        {
            case 0: m_color_mode = COLOR_MODE_GRAYSCALE; break;
            case 1: m_color_mode = COLOR_MODE_8BIT;      break;
            case 2: m_color_mode = COLOR_MODE_HSV;       break;
            default: return -EINVAL;
        }

        param = params[3];
        m_track_changes = (param.toUInt(&ok) != 0);
        if (!ok) return -EINVAL;

        param = params[4];
        m_follow_selection = (param.toUInt(&ok) != 0);
        if (!ok) return -EINVAL;

        return 0;
    } else {
        qWarning("SonagramPlugin::interpreteParameters(): "
                 "invalid parameter list: %s",
                 DBG(params.join(_(", "))));
    }

    return -EINVAL;
}

//***************************************************************************
int Kwave::SonagramPlugin::start(QStringList &params)
{
    // clean up leftovers from last run
    delete m_sonagram_window;
    m_sonagram_window = nullptr;
    delete m_selection;
    m_selection = nullptr;
    delete m_overview_cache;
    m_overview_cache = nullptr;

    Kwave::SignalManager &sig_mgr = signalManager();

    // interpret parameter list and abort if it contains invalid data
    int result = interpreteParameters(params);
    if (result) return result;

    // create an empty sonagram window
    m_sonagram_window = new(std::nothrow)
        Kwave::SonagramWindow(parentWidget());
    Q_ASSERT(m_sonagram_window);
    if (!m_sonagram_window) return -ENOMEM;
    if (m_mode == MODE_LOAD) {
        m_sonagram_window->setWindowTitle(
            i18n("Sonagram loaded from %1", params[1]));
    } else {
        m_sonagram_window->setName(signalName());
    }

    // if the signal closes, close the sonagram window too
    QObject::connect(&manager(), SIGNAL(sigClosed()),
                     m_sonagram_window, SLOT(close()));

    // forward the (menu) commands to the plugin
    QObject::connect(m_sonagram_window, SIGNAL(sigCommand(QString)),
                     this, SLOT(emitCommand(QString)));

    if (m_mode == MODE_VIEW) {
        // get the current selection
        QVector<unsigned int> selected_channels;
        sample_index_t offset = 0;
        sample_index_t length = 0;
        length = selection(&selected_channels, &offset, nullptr, true);

        // abort if nothing is selected
        if (!length || selected_channels.isEmpty())
            return -EINVAL;

        // calculate the number of slices (width of image)
        m_slices = Kwave::toUint(ceil(static_cast<double>(length) /
                                    static_cast<double>(m_fft_points)));
        if (m_slices > MAX_SLICES) m_slices = MAX_SLICES;

        /* limit selection to INT_MAX samples (limitation of the cache index) */
        if ((length / m_fft_points) >= SAMPLE_INDEX_MAX) {
            Kwave::MessageBox::error(parentWidget(),
                                    i18n("File or selection too large"));
            return -EFBIG;
        }

        // create a selection tracker
        m_selection = new(std::nothrow) Kwave::SelectionTracker(
            &sig_mgr, offset, length, &selected_channels);
        Q_ASSERT(m_selection);
        if (!m_selection) return -ENOMEM;

        connect(m_selection, SIGNAL(sigTrackInserted(quint64)),
                this,        SLOT(slotTrackInserted(quint64)));
        connect(m_selection, SIGNAL(sigTrackDeleted(quint64)),
                this,        SLOT(slotTrackDeleted(quint64)));
        connect(
            m_selection,
            SIGNAL(sigInvalidated(quint64,sample_index_t,sample_index_t)),
            this,
            SLOT(slotInvalidated(quint64,sample_index_t,sample_index_t))
        );

        // create a new empty image
        createNewImage(m_slices, m_fft_points / 2);

        // set the overview
        m_overview_cache = new(std::nothrow)
            Kwave::OverViewCache(sig_mgr, offset, length, &selected_channels);
        Q_ASSERT(m_overview_cache);
        if (!m_overview_cache) return -ENOMEM;

        refreshOverview(); // <- this needs the m_overview_cache

        if (m_track_changes && (m_mode == MODE_VIEW)) {
            // stay informed about changes in the signal
            connect(m_overview_cache, SIGNAL(changed()),
                    this, SLOT(refreshOverview()));
        } else {
            // overview cache is no longer needed
            delete m_overview_cache;
            m_overview_cache = nullptr;
        }
    } else if ((m_mode == MODE_LOAD) && (params.count() >= 2)) {
        // load the sonagram from a file
        result = loadFromFile(params[1]);
        if (result) return result;
    } else if ((m_mode == MODE_SAVE) && (params.count() >= 3)) {
        // save the sonagram to a file
        bool ok = false;
        quint64 id = params[1].toULongLong(&ok);
        if (!ok) return -EINVAL;
        result = saveToFile(id, params[2]);
        return result;
    }

    // connect all needed signals
    connect(m_sonagram_window, SIGNAL(destroyed()),
            this, SLOT(windowDestroyed()));

    // activate the window with an initial image
    // and all necessary information
    m_sonagram_window->setColorMode(m_color_mode);
    m_sonagram_window->setImage(m_image);
    m_sonagram_window->setPoints(m_fft_points);
    m_sonagram_window->setRate(signalRate());
    m_sonagram_window->show();

    if (m_track_changes && (m_mode == MODE_VIEW)) {
        QObject::connect(static_cast<QObject*>(&(manager())),
            SIGNAL(sigSignalNameChanged(QString)),
            m_sonagram_window, SLOT(setName(QString)));
    }

    // increment the usage counter and release the plugin when the
    // sonagram window closed
    use();

    return 0;
}

//***************************************************************************
void Kwave::SonagramPlugin::makeAllValid()
{
    unsigned int             fft_points;
    unsigned int             slices;
    Kwave::window_function_t window_type;
    sample_index_t           first_sample;
    sample_index_t           last_sample;
    QBitArray                valid;
    QVector<unsigned int>    track_list;

    {
        QMutexLocker _lock(&m_lock_job_list);

        if (!m_selection) return;
        if (!m_selection->length() || (m_fft_points < 4)) return;

        fft_points   = m_fft_points;
        slices       = m_slices;
        window_type  = m_window_type;
        first_sample = m_selection->first();
        last_sample  = m_selection->last();
        valid        = m_valid;
        m_valid.fill(true);

        const QList<quint64> selected_tracks(m_selection->allTracks());
        for (unsigned int track : signalManager().allTracks())
            if (selected_tracks.contains(signalManager().uidOfTrack(track)))
                track_list.append(track);
    }
    const unsigned int tracks = static_cast<unsigned int>(track_list.count());

    Kwave::WindowFunction func(window_type);
    const QVector<double> windowfunction = func.points(fft_points);
    Q_ASSERT(windowfunction.count() == Kwave::toInt(fft_points));
    if (windowfunction.count() != Kwave::toInt(fft_points)) return;

    Kwave::MultiTrackReader source(Kwave::SinglePassForward,
        signalManager(), track_list, first_sample, last_sample);

//     qDebug("SonagramPlugin[%p]::makeAllValid() [%llu .. %llu]",
//      static_cast<void *>(this), first_sample, last_sample);

    QColor transparent;
    switch (m_color_mode) {
        case COLOR_MODE_GRAYSCALE: /* FALLTHROUGH */
        case COLOR_MODE_8BIT: transparent.setRgb(0xFF);    break;
        case COLOR_MODE_HSV:  transparent.setHsv(0, 0, 0); break;
        default: break;
    }

    QFutureSynchronizer<void> synchronizer;
    for (unsigned int slice_nr = 0; slice_nr < slices; slice_nr++) {
//      qDebug("SonagramPlugin::run(): calculating slice %d of %d",
//             slice_nr, m_slices);

        if (valid[slice_nr]) continue;

        // determine start of the stripe
        sample_index_t pos = first_sample + (slice_nr * fft_points);

        // get a new slice from the pool and initialize it
        Kwave::SonagramPlugin::Slice *slice = m_slice_pool.allocate();
        Q_ASSERT(slice);

        slice->m_index = slice_nr;
        memset(slice->m_input,  0x00, sizeof(slice->m_input));
        memset(slice->m_output, 0x00, sizeof(slice->m_output));

        // initialize result with zeroes
        slice->m_result.fill(transparent);
        if ((pos <= last_sample) && (tracks)) {

            // seek to the start of the slice
            source.seek(pos);

            // we have a new slice, now fill it's input buffer
            double *in = slice->m_input;
            for (unsigned int j = 0; j < fft_points; j++) {
                double value = 0.0;
                if (!(source.eof())) {
                    for (unsigned int t = 0; t < tracks; t++) {
                        sample_t s = 0;
                        Kwave::SampleReader *reader = source[t];
                        Q_ASSERT(reader);
                        if (reader) *reader >> s;
                        value += sample2double(s);
                    }
                    value /= tracks;
                }
                in[j] = value * windowfunction[j];
            }

            // a background job is running soon
            // (for counterpart, see insertSlice(...) below [main thread])
            m_pending_jobs.lockForRead();

            // run the FFT in a background thread
            synchronizer.addFuture(QtConcurrent::run(
                &Kwave::SonagramPlugin::calculateSlice, this, slice)
            );
        } else {
            // range has been deleted -> keep filled with "empty"
            m_pending_jobs.lockForRead();
            emit sliceAvailable(slice);
        }

        if (shouldStop()) break;
    }

//     qDebug("SonagramPlugin::makeAllValid(): waiting for background jobs...");

    // wait for all worker threads
    synchronizer.waitForFinished();

    // wait for queued signals
    m_pending_jobs.lockForWrite();
    m_pending_jobs.unlock();

//     qDebug("SonagramPlugin::makeAllValid(): done.");
}

//***************************************************************************
void Kwave::SonagramPlugin::run(QStringList params)
{
    qDebug("SonagramPlugin::run()");
    Q_UNUSED(params)
    {
        // invalidate all slices
        QMutexLocker _lock(&m_lock_job_list);
        m_valid.fill(false);
    }
    makeAllValid();
}

//***************************************************************************
void Kwave::SonagramPlugin::calculateSlice(Kwave::SonagramPlugin::Slice *slice)
{
    fftw_plan p;

    // prepare for a 1-dimensional real-to-complex DFT
    {
        Kwave::GlobalLock _lock; // libfftw is not threadsafe!
        p = fftw_plan_dft_r2c_1d(
            m_fft_points,
            &(slice->m_input[0]),
            &(slice->m_output[0]),
            FFTW_ESTIMATE
        );
    }
    Q_ASSERT(p);
    if (!p) return;

    // calculate the fft (according to the specs, this is the one and only
    // libfft function that is threadsafe!)
    fftw_execute(p);

    // norm all values to [0...254] and use them as pixel value
    for (unsigned int j = 0; j < m_fft_points / 2; j++) {
        // get signal energy and scale to [0 .. 1]
        std::complex<double> z{slice->m_output[j][0], slice->m_output[j][1]};
        double magnitude = std::abs(z) / static_cast<double>(m_fft_points / 2);
        const double minDb = -60.0;
        double db = 20.0 * std::log10(std::max(magnitude, 1E-6));
        double a = (db - minDb) / (-minDb);
        a = qBound(0.0, a, 1.0);

        QColor c{};
        switch (m_color_mode)
        {
            case COLOR_MODE_GRAYSCALE:
            case COLOR_MODE_8BIT: /* FALLTHROUGH */
                // grayscale or rainbow effect (indexed)
                c.setRgb(static_cast<quint8>(a * 254.0), 0, 0, 255);
                break;
            case COLOR_MODE_HSV:
            {
                // rainbow effect + angle
                float h = static_cast<float>((std::arg(z) + M_PI) / (2 * M_PI));
                float s = static_cast<float>(1.0);
                float v = static_cast<float>(a);
                c.setHsvF(s, h, v);
                break;
            }
            default: break;
        }
        slice->m_result[j] = c;
    }

    // free the allocated FFT resources
    {
        Kwave::GlobalLock _lock; // libfftw is not threadsafe!
        fftw_destroy_plan(p);
    }

    // emit the slice data to be synchronously inserted into
    // the current image in the context of the main thread
    // (Qt does the queuing for us)
    emit sliceAvailable(slice);
}

//***************************************************************************
void Kwave::SonagramPlugin::insertSlice(Kwave::SonagramPlugin::Slice *slice)
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    Q_ASSERT(slice);
    if (!slice) return;


    // forward the slice to the window to display it
    if (m_sonagram_window)
        m_sonagram_window->insertSlice(slice->m_index, slice->m_result);

    // return the slice into the pool
    m_slice_pool.release(slice);

    // job is done
    m_pending_jobs.unlock();
}

//***************************************************************************
void Kwave::SonagramPlugin::createNewImage(const unsigned int width,
                                           const unsigned int height)
{
    // delete the previous image
    m_image = QImage();
    if (m_sonagram_window) m_sonagram_window->setImage(m_image);

    // do not create a new image if one dimension is zero!
    Q_ASSERT(width);
    Q_ASSERT(height);
    if (!width || !height) return;

    // also do not create if the image size is out of range
    Q_ASSERT(width <= 32767);
    Q_ASSERT(height <= 32767);
    if ((width >= 32767) || (height >= 32767)) return;

    // create the new image object
    QImage::Format format;
    switch (m_color_mode)
    {
        case COLOR_MODE_GRAYSCALE: /* FALLTHROUGH */
        case COLOR_MODE_8BIT:
            format = QImage::Format_Indexed8;
            break;
        case COLOR_MODE_HSV:
            format = QImage::Format_RGBA8888;
            break;
        default:
            return;
    }
    m_image = QImage(width, height, format);
    Q_ASSERT(!m_image.isNull());
    if (m_image.isNull()) return;

    // initialize the image's palette with transparency
    if (m_color_mode != COLOR_MODE_HSV) {
        m_image.setColorCount(256);
        for (int i = 0; i < 256; i++) {
            m_image.setColor(i, 0x00000000);
        }

        // fill the image with "empty" (transparent)
        m_image.fill(0xFF);
    }

}

//***************************************************************************
void Kwave::SonagramPlugin::refreshOverview()
{
    if (!m_overview_cache || !m_sonagram_window) return;

    QColor fg = m_sonagram_window->palette().light().color();
    QColor bg = m_sonagram_window->palette().mid().color();
    QImage overview = m_overview_cache->getOverView(
       m_sonagram_window->width(), SONAGRAM_OVERVIEW_HEIGHT, fg, bg);

    m_sonagram_window->setOverView(overview);
}

//***************************************************************************
void Kwave::SonagramPlugin::requestValidation()
{
    // only re-start the repaint timer, this hides some GUI update artifacts
    if (!m_repaint_timer.isActive()) {
        m_repaint_timer.stop();
        m_repaint_timer.setSingleShot(true);
        m_repaint_timer.start(REPAINT_INTERVAL);
    }
}

//***************************************************************************
void Kwave::SonagramPlugin::validate()
{
    // wait for previously running jobs to finish
    if (m_future.isRunning()) {
        requestValidation();
        return; // job is still running, come back later...
    }

    // queue a background thread for updates
    m_future = QtConcurrent::run(&Kwave::SonagramPlugin::makeAllValid, this);
}

//***************************************************************************
void Kwave::SonagramPlugin::slotTrackInserted(quint64 track_id)
{
    QMutexLocker _lock(&m_lock_job_list);

    Q_UNUSED(track_id)

    // check for "track changes" mode
    if (!m_track_changes) return;

    // invalidate complete signal
    m_valid.fill(false, m_slices);
    requestValidation();
}

//***************************************************************************
void Kwave::SonagramPlugin::slotTrackDeleted(quint64 track_id)
{
    QMutexLocker _lock(&m_lock_job_list);

    Q_UNUSED(track_id)

    // check for "track changes" mode
    if (!m_track_changes) return;

    // invalidate complete signal
    m_valid.fill(false, m_slices);
    requestValidation();
}

//***************************************************************************
void Kwave::SonagramPlugin::slotInvalidated(quint64 track_id,
                                            sample_index_t first,
                                            sample_index_t last)
{
    QMutexLocker lock(&m_lock_job_list);

    Q_UNUSED(track_id)
//     qDebug("SonagramPlugin[%p]::slotInvalidated(%s, %llu, %llu)",
//          static_cast<void *>(this),
//         (track_id) ? DBG(track_id->toString()) : "*", first, last);

    // check for "track changes" mode
    if (!m_track_changes) return;

    // adjust offsets, absolute -> relative
    sample_index_t offset = (m_selection) ? m_selection->offset() : 0;
    Q_ASSERT(first >= offset);
    Q_ASSERT(last  >= offset);
    Q_ASSERT(last  >= first);
    first -= offset;
    last  -= offset;

    unsigned int first_idx = Kwave::toUint(first / m_fft_points);
    unsigned int last_idx;
    if (last >= (SAMPLE_INDEX_MAX - (m_fft_points - 1)))
        last_idx = m_slices - 1;
    else
        last_idx = Kwave::toUint(qMin(Kwave::round_up(last,
            static_cast<sample_index_t>(m_fft_points)) / m_fft_points,
            static_cast<sample_index_t>(m_slices - 1))
        );

    m_valid.fill(false, first_idx, last_idx + 1);
    requestValidation();
}

//***************************************************************************
void Kwave::SonagramPlugin::windowDestroyed()
{
    cancel();

    m_sonagram_window = nullptr; // closes itself !

    delete m_selection;
    m_selection = nullptr;

    delete m_overview_cache;
    m_overview_cache = nullptr;

    release();
}

//***************************************************************************
int Kwave::SonagramPlugin::loadFromFile(const QString &filename)
{
    qDebug("loading %s", DBG(filename));
    if (filename.isEmpty()) return -EINVAL;

    QImage image(filename);
    if (image.isNull()) return -ENOENT;

    // detect the color mode
    if (image.colorCount() >= 255) {
        m_color_mode = COLOR_MODE_GRAYSCALE;
        for (const QColor c : image.colorTable()) {
            if ((c.red() != c.green()) || (c.green() != c.blue())) {
                m_color_mode = COLOR_MODE_8BIT;
                break;
            }
        }
    } else
        m_color_mode = COLOR_MODE_HSV;

    m_slices     = image.width();
    m_fft_points = image.height() * 2;

    // try to find out the sample rate
    if (m_sonagram_window) {
        // try to find it in file meta data
        bool ok = false;
        double rate = m_image.text(_("x-kwave/sample-rate")).toDouble(&ok);
        if (!ok) {
            // fallback #1: use dpi setting
            if (m_image.dotsPerMeterX() == m_image.dotsPerMeterY()) {
                rate = m_image.dotsPerMeterX();
                ok = !qFuzzyIsNull(rate) && (rate >= 8000.0);
            }
        }
        if (!ok) {
            // fallback #2: use current signal
            Kwave::SignalManager &mgr = signalManager();
            rate = mgr.rate();
            ok = !qFuzzyIsNull(rate) && (rate >= 8000.0);
        }
        if (!ok) {
            // fallback #3: assume some default rate
            rate = 44100.0;
        }
        m_sonagram_window->setRate(rate);
        m_sonagram_window->setColorMode(m_color_mode);
    }

    m_image = std::move(image);
    return 0;
}

//***************************************************************************
int Kwave::SonagramPlugin::saveToFile(quint64 index, const QString &filename)
{
    qDebug("saving sonagram #%llu to %s", index, DBG(filename));
    if (filename.isEmpty()) return -EINVAL;

    // find the sonagram window with the given index
    Kwave::SonagramWindow *win = Kwave::SonagramWindow::fromIndex(index);
    if (win == nullptr) return -EINVAL;

    // get a copy of the image
    QImage image(win->image());

    // attach some meta data to the image for loading it later
    double rate = win->rate();
    if (qFuzzyIsNull(rate) || (rate < 8000.0))
        rate = signalRate();
    if (!qFuzzyIsNull(rate) && (rate >= 8000.0)) {
        // try to save as file meta data
        image.setText(_("x-kwave/sample-rate"), QString::number(rate));
        // also save as dpi as fallback
        image.setDotsPerMeterX(Kwave::toInt(rate));
        image.setDotsPerMeterY(Kwave::toInt(rate));
    }

    return (image.save(filename, "BMP")) ? 0 : -EIO;
}

//***************************************************************************
#include "SonagramPlugin.moc"
//***************************************************************************
//***************************************************************************
