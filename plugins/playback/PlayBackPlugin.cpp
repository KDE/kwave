/***************************************************************************
     PlayBackPlugin.cpp  -  plugin for playback and playback configuration
			     -------------------
    begin                : Sun May 13 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <qarray.h>
#include <qvector.h>
#include <qstring.h>

#include <kapp.h>

#include <libkwave/Matrix.h>
#include <libkwave/Sample.h>
#include <libkwave/SampleReader.h>

#include <libgui/KwavePlugin.h>

#include <kwave/PlaybackController.h>
#include <kwave/PluginManager.h>
#include <kwave/SignalManager.h>

#include "PlayBackDialog.h"
#include "PlayBackPlugin.h"

KWAVE_PLUGIN(PlayBackPlugin,"playback","Thomas Eschenbacher");

/** Sets the number of screen refreshes per second when in playback mode */
#define SCREEN_REFRESHES_PER_SECOND 15

#define DEFAULT_PLAYBACK_DEVICE "/dev/dsp"
// "[aRts sound daemon]"

/** use at least 2^8 = 256 bytes for playback buffer !!! */
#define MIN_PLAYBACK_BUFFER 8

/** use at most 2^16 = 65536 bytes for playback buffer !!! */
#define MAX_PLAYBACK_BUFFER 16

//***************************************************************************
PlayBackPlugin::PlayBackPlugin(PluginContext &context)
    :KwavePlugin(context),
    m_playback_controller(manager().playbackController()),
    m_spx_playback_done(&m_playback_controller, SLOT(playbackDone())),
    m_spx_playback_pos(this, SLOT(updatePlaybackPos())),
    m_stop(false)
{
    m_playback_params.rate = 44100;
    m_playback_params.channels = 2;
    m_playback_params.bits_per_sample = 16;
    m_playback_params.device = DEFAULT_PLAYBACK_DEVICE;
    m_playback_params.bufbase = 10;

    m_spx_playback_pos.setLimit(32); // limit for the queue
}

//***************************************************************************
int PlayBackPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    ASSERT(params.count() == 4);
    if (params.count() != 4) {
	debug("PlayBackPlugin::interpreteParams(): params.count()=%d",
	      params.count());
	return -EINVAL;
    }

    // parameter #0: number of channels [1 | 2]
    param = params[0];
    m_playback_params.channels = param.toUInt(&ok);
    ASSERT(ok);
    if (!ok) return -EINVAL;

    // parameter #1: bits per sample [8 | 16 ]
    param = params[1];
    m_playback_params.bits_per_sample = param.toUInt(&ok);
    ASSERT(ok);
    if (!ok) return -EINVAL;

    // parameter #2: playback device [/dev/dsp , ... ]
    param = params[2];
    m_playback_params.device = param;

    // parameter #3: base of buffer size [4...16]
    param = params[3];
    m_playback_params.bufbase = param.toUInt(&ok);
    ASSERT(ok);
    if (!ok) return -EINVAL;

    return 0;
}

//***************************************************************************
void PlayBackPlugin::load(QStringList &params)
{
    interpreteParameters(params);

    connect(&m_playback_controller, SIGNAL(sigDeviceStartPlayback()),
            this, SLOT(startDevicePlayBack()));
    connect(&m_playback_controller, SIGNAL(sigDeviceStopPlayback()),
            this, SLOT(stopDevicePlayBack()));
}

//***************************************************************************
QStringList *PlayBackPlugin::setup(QStringList &previous_params)
{
    QStringList *result = 0;

    // try to interprete the list of previous parameters, ignore errors
    if (previous_params.count()) interpreteParameters(previous_params);

    PlayBackDialog *dlg = new PlayBackDialog(*this, m_playback_params);
    ASSERT(dlg);
    if (!dlg) return 0;

    if (dlg->exec() == QDialog::Accepted) {
	// get the new parameters and let them take effect
	result = new QStringList();
	ASSERT(result);
	if (result) {
	    dlg->parameters(*result);
	    interpreteParameters(*result);
	}
    };

    delete dlg;
    return result;
}

//***************************************************************************
void PlayBackPlugin::startDevicePlayBack()
{
    static QStringList empty_list;

    // determine first and last sample if not in paused mode
    if (!m_playback_controller.paused()) {
	unsigned int first;
	unsigned int last;
	selection(&first, &last);
	if (first == last) {
	    // nothing selected -> play everything
	    first = 0;
	    last  = signalLength()-1;
	}
	m_playback_controller.setStartPos(first);
	m_playback_controller.setEndPos(last);
	m_playback_controller.updatePlaybackPos(first);
    }

    // set the real sample rate for playback from the signal itself
    m_playback_params.rate = signalRate();

    m_stop = false;
    execute(empty_list);
}

//***************************************************************************
void PlayBackPlugin::stopDevicePlayBack()
{
    m_stop = true;
}

//***************************************************************************
int PlayBackPlugin::setSoundParams(int audio, int bitspersample,
                                   unsigned int channels, int rate,
                                   int bufbase)
{
    QString m_playback_error; // ### dummy

    debug("PlayBackPlugin::setSoundParams(fd=%d,bps=%d,channels=%d,"\
	"rate=%d, bufbase=%d)", audio, bitspersample, channels,
	rate, bufbase);

    int format = (bitspersample == 8) ? AFMT_U8 : AFMT_S16_LE;

    // number of bits per sample
    if (ioctl(audio, SNDCTL_DSP_SAMPLESIZE, &format) == -1) {
	m_playback_error = i18n("number of bits per samples not supported");
	return 0;
    }

    // mono/stereo selection
    int stereo = (channels >= 2) ? 1 : 0;
    if (ioctl(audio, SNDCTL_DSP_STEREO, &stereo) == -1) {
	m_playback_error = i18n("stereo not supported");
	return 0;
    }

    // playback rate
    if (ioctl(audio, SNDCTL_DSP_SPEED, &rate) == -1) {
	m_playback_error = i18n("playback rate not supported");
	return 0;
    }

    // buffer size
    ASSERT(bufbase >= MIN_PLAYBACK_BUFFER);
    ASSERT(bufbase <= MAX_PLAYBACK_BUFFER);
    if (bufbase < MIN_PLAYBACK_BUFFER) bufbase = MIN_PLAYBACK_BUFFER;
    if (bufbase > MAX_PLAYBACK_BUFFER) bufbase = MAX_PLAYBACK_BUFFER;
    if (ioctl(audio, SNDCTL_DSP_SETFRAGMENT, &bufbase) == -1) {
	m_playback_error = i18n("unusable buffer size");
	return 0;
    }

    // return the buffer size in bytes
    int size;
    ioctl(audio, SNDCTL_DSP_GETBLKSIZE, &size);
    return size;
}

//***************************************************************************
void PlayBackPlugin::run(QStringList)
{
    unsigned int first = m_playback_controller.startPos();
    unsigned int last  = m_playback_controller.endPos();
    unsigned int out_channels = m_playback_params.channels;

    // get the list of selected channels
    QArray<unsigned int> audible_tracks = selectedTracks();
    unsigned int audible_count = audible_tracks.count();
    if (!audible_count) {
	// not even one selected track
	debug("PlayBackPlugin::run(): no audible track(s) !");
	playbackDone();
	return;
    }

    // set up a set of sample reader (streams)
    QVector<SampleReader> input;
    openSampleReaderSet(input, audible_tracks, first, last);

    // create a translation matrix for mixing up/down to the desired
    // number of output channels
    Matrix<double> matrix(audible_count, out_channels);
    unsigned int x, y;
    for (y=0; y < out_channels; y++) {
	unsigned int m1, m2;
	m1 = y * audible_count;
	m2 = (y+1) * audible_count;
	
	for (x=0; x < audible_count; x++) {
	    unsigned int n1, n2;
	    n1 = x * out_channels;
	    n2 = n1 + out_channels;
	
	    // get the common area of [n1..n2] and [m1..m2]
	    unsigned int l  = max(n1, m1);
	    unsigned int r = min(n2, m2);
	
	    matrix[x][y] = (r > l) ?
		(double)(r-l) / (double)audible_count : 0.0;
	}
    }

    // prepeare for playback by opening the sound device
    // and initializing with the proper settings
    int device;
    unsigned int bufsize = 0;
    if ((device = open(m_playback_params.device, O_WRONLY)) != -1 ) {
	bufsize = setSoundParams(device,
	    m_playback_params.bits_per_sample,
	    m_playback_params.channels,
	    m_playback_params.rate,
	    m_playback_params.bufbase
	);
    }

    QByteArray buffer(bufsize);

    // loop until process is stopped
    // or run once if not in loop mode
    QArray<sample_t> in_samples(audible_count);
    QArray<sample_t> out_samples(out_channels);
    unsigned int pointer = m_playback_controller.currentPos();

    // counter for refresh of the playback position
    unsigned int pos_countdown = 0;

    m_spx_playback_pos.enqueue(pointer);
    do {

	// if current position is after start -> skip the passed
	// samples (this happens when resuming after a pause)
	if (pointer > first) {
	    for (x=0; x < audible_count; x++) {
		SampleReader *stream = input[x];
		if (stream) stream->skip(pointer-first);
	    }
	}
	
	while ((pointer <= last) && !m_stop) {
	    // fill the buffer with audio data
	    unsigned int cnt;
	    for (cnt = 0; (cnt < bufsize) && (pointer <= last); pointer++) {
		// read vector with input samples
		unsigned int x;
		for (x=0; x < audible_count; x++) {
		    in_samples[x] = 0;
		    SampleReader *stream = input[x];
		    ASSERT(stream);
		    if (!stream) continue;
		
		    sample_t act;
		    (*stream) >> act;
		    in_samples[x] = act;
		}
		
		// multiply matrix with input to get output
		unsigned int y;
		for (y=0; y < out_channels; y++) {
		    double sum = 0;
		    for (x=0; x < audible_count; x++) {
			sum += (double)in_samples[x] * matrix[x][y];
		    }
		    out_samples[y] = (sample_t)sum;
		}
		
		// convert into byte stream
		unsigned int channel;
		for (channel=0; channel < out_channels; channel++) {
		    sample_t sample = out_samples[channel];
		
		    switch (m_playback_params.bits_per_sample) {
			case 8:
			    sample += 1 << 23;
			    buffer[cnt++] = sample >> 16;
			    break;
			case 16:
			    sample += 1 << 23;
			    buffer[cnt++] = sample >> 8;
			    buffer[cnt++] = (sample >> 16) + 128;
			    break;
			case 24:
			    // play in 32 bit format
			    buffer[cnt++] = 0x00;
			    buffer[cnt++] = sample & 0x0FF;
			    buffer[cnt++] = sample >> 8;
			    buffer[cnt++] = (sample >> 16) + 128;
			    break;
			default:
			    // invalid bits per sample
			    m_stop = true;
			    pointer = last;
			    cnt = 0;
			    break;
		    }
		}
	    }
	
	    // write buffer to the playback device
	    write(device, buffer.data(), cnt);
	
	    // update the playback position if timer elapsed
	    if (pos_countdown < cnt) {
		pos_countdown = m_playback_params.rate *
		    out_channels * (m_playback_params.bits_per_sample >> 3)
		    / SCREEN_REFRESHES_PER_SECOND;
		m_spx_playback_pos.enqueue(pointer);
	    } else {
		pos_countdown -= cnt;
	    }
	}
	
	// maybe we loop. in this case the playback starts
	// again from the left marker
	if (m_playback_controller.loop() && !m_stop) {
	    for (x=0; x < audible_count; x++) {
		SampleReader *stream = input[x];
		if (stream) stream->reset();
	    }
	    pointer = m_playback_controller.startPos();
	}

    } while (m_playback_controller.loop() && !m_stop);

    // finish the output
    ::close(device);

    // the playback buffer MUST NOT be freed while the playback (device) is
    // still using it, so resize it right here, after closing the device.
    buffer.resize(0);

    // playback is done
    debug("PlayBackPlugin::run(): playback done");
    playbackDone();
}

//***************************************************************************
void PlayBackPlugin::updatePlaybackPos()
{
    unsigned int count = m_spx_playback_pos.count();
    unsigned int pos = 0;

    // dequeue all pointers and keep the latest one
    if (!count) return;
    while (count--) {
	unsigned int *ppos = m_spx_playback_pos.dequeue();
	ASSERT(ppos);
	if (!ppos) continue;
	pos = *ppos;
	delete ppos;
    }

   m_playback_controller.updatePlaybackPos(pos);
}

//***************************************************************************
void PlayBackPlugin::playbackDone()
{
    // generate a "playback done" event through the signal proxy
    m_spx_playback_done.AsyncHandler();
}

//***************************************************************************
//***************************************************************************
