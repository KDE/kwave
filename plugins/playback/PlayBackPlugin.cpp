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

#include <qstring.h>

#include <kapp.h>

#include <libgui/KwavePlugin.h>

#include <kwave/PlaybackController.h>
#include <kwave/PluginManager.h>
#include <kwave/SignalManager.h>

#include "PlayBackDialog.h"
#include "PlayBackPlugin.h"

KWAVE_PLUGIN(PlayBackPlugin,"playback","Thomas Eschenbacher");

#define DEFAULT_PLAYBACK_DEVICE "[aRts sound daemon]"

//***************************************************************************
PlayBackPlugin::PlayBackPlugin(PluginContext &context)
    :KwavePlugin(context)
{
    m_playback_params.rate = 44100;
    m_playback_params.channels = 2;
    m_playback_params.bits_per_sample = 16;
    m_playback_params.device = DEFAULT_PLAYBACK_DEVICE;
    m_playback_params.bufbase = 10;

}
//***************************************************************************
int PlayBackPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;
    debug("PlayBackPlugin::interpreteParameters()"); // ###

    // evaluate the parameter list
    ASSERT(params.count() == 5);
    if (params.count() != 5) {
	debug("PlayBackPlugin::interpreteParams(): params.count()=%d",
	      params.count());
	return -EINVAL;
    }
	
    // parameter #0: sample rate [44100]
    param = params[0];
    m_playback_params.rate = param.toUInt(&ok);
    ASSERT(ok);
    if (!ok) return -EINVAL;

    // parameter #1: number of channels [1 | 2]
    param = params[1];
    m_playback_params.channels = param.toUInt(&ok);
    ASSERT(ok);
    if (!ok) return -EINVAL;

    // parameter #2: bits per sample [8 | 16 ]
    param = params[2];
    m_playback_params.bits_per_sample = param.toUInt(&ok);
    ASSERT(ok);
    if (!ok) return -EINVAL;

    // parameter #3: playback device [/dev/dsp , ... ]
    param = params[3];
    m_playback_params.device = param;

    // parameter #4: base of buffer size [4...16]
    param = params[4];
    m_playback_params.bufbase = param.toUInt(&ok);
    ASSERT(ok);
    if (!ok) return -EINVAL;

    debug("PlayBackPlugin::interpreteParameters(): done"); // ###
    return 0;
}
//***************************************************************************
void PlayBackPlugin::load(QStringList &params)
{
    debug("PlaybackPlugin(): load()");
    interpreteParameters(params);

    PlaybackController &m_playback_controller =
	manager().playbackController();

    connect(&m_playback_controller, SIGNAL(sigDeviceStartPlayback()),
            this, SLOT(startDevicePlayBack()));
    connect(&m_playback_controller, SIGNAL(sigDeviceStopPlayback()),
            this, SLOT(stopDevicePlayBack()));
}

//***************************************************************************
QStringList *PlayBackPlugin::setup(QStringList &previous_params)
{
    QStringList *result = 0;
    debug("SonagramPlugin::setup()");

    // try to interprete the list of previous parameters, ignore errors
    if (previous_params.count()) interpreteParameters(previous_params);

    PlayBackDialog *dlg = new PlayBackDialog(*this, m_playback_params);
    ASSERT(dlg);
    if (!dlg) return 0;

    if (dlg->exec() == QDialog::Accepted) {
	result = new QStringList();
	ASSERT(result);
	if (result) dlg->parameters(*result);
    };

    delete dlg;
    debug("PlayBackPlugin::setup(): done.");
    return result;
}

//***************************************************************************
void PlayBackPlugin::startDevicePlayBack()
{
    debug("PlayBackPlugin::startDevicePlayBack()()");
//    msg[processid] = 1;
//    msg[stopprocess] = false;
//
//    Play *par = new Play;
//    pthread_t thread;
//
//    par->manage = this;
//    par->start = start;
//    par->loop = loop;
//    par->params = KwaveApp::getPlaybackParams();
//    par->params.rate = this->getRate();
//
//    m_playback_error = 0;
//    pthread_create(&thread, 0, (void * (*) (void *))playThread, par);
}

//***************************************************************************
void PlayBackPlugin::stopDevicePlayBack()
{
    debug("PlayBackPlugin::stopDevicePlayBack()");
//    msg[stopprocess] = true;          //set flag for stopping
//
//    QTimer timeout;
//    timeout.start(5000, true);
//    while (msg[processid] != 0) {
//	sched_yield();
//	// wait for termination
//	if (!timeout.isActive()) {
//	    warning("SignalManager::stopplay(): TIMEOUT");
//	    break;
//	}
//    }
}

//***************************************************************************
//int PlayBackPlugin::setSoundParams(int /*audio*/, int /*bitspersample*/,
//                                   unsigned int /*channels*/, int /*rate*/,
//                                   int /*bufbase*/)
//{
//    return 0;
//    const char *trouble = i18n("playback problem");
//
//    debug("SignalManager::setSoundParams(fd=%d,bps=%d,channels=%d,"\#
//	"rate=%d, bufbase=%d)", audio, bitspersample, channels,
//	rate, bufbase);
//
//// ### under construction ###
//
//// from standard oss interface (linux/soundcard.h)
//
/////*	Audio data formats (Note! U8=8 and S16_LE=16 for compatibility) */
////#define SNDCTL_DSP_GETFMTS		_SIOR ('P',11, int) /* Returns a mask */
////#define SNDCTL_DSP_SETFMT		_SIOWR('P',5, int) /* Selects ONE fmt*/
////#	define AFMT_QUERY		0x00000000	/* Return current fmt */
////#	define AFMT_MU_LAW		0x00000001
////#	define AFMT_A_LAW		0x00000002
////#	define AFMT_IMA_ADPCM		0x00000004
////#	define AFMT_U8			0x00000008
////#	define AFMT_S16_LE		0x00000010	/* Little endian signed 16*/
////#	define AFMT_S16_BE		0x00000020	/* Big endian signed 16 */
////#	define AFMT_S8			0x00000040
////#	define AFMT_U16_LE		0x00000080	/* Little endian U16 */
////#	define AFMT_U16_BE		0x00000100	/* Big endian U16 */
////#	define AFMT_MPEG		0x00000200	/* MPEG (2) audio */
//
//// from ALSA interface (asound.h)
//
////#define SND_PCM_SFMT_S8			0
////#define SND_PCM_SFMT_U8			1
////#define SND_PCM_SFMT_S16		SND_PCM_SFMT_S16_LE
////#define SND_PCM_SFMT_U16		SND_PCM_SFMT_U16_LE
////#define SND_PCM_SFMT_S24		SND_PCM_SFMT_S24_LE
////#define SND_PCM_SFMT_U24		SND_PCM_SFMT_U24_LE
////#define SND_PCM_SFMT_S32		SND_PCM_SFMT_S32_LE
////#define SND_PCM_SFMT_U32		SND_PCM_SFMT_U32_LE
////#define SND_PCM_SFMT_FLOAT		SND_PCM_SFMT_FLOAT_LE
////#define SND_PCM_SFMT_FLOAT64		SND_PCM_SFMT_FLOAT64_LE
////#define SND_PCM_SFMT_IEC958_SUBFRAME	SND_PCM_SFMT_IEC958_SUBFRAME_LE
//
//    int format = (bitspersample == 8) ? AFMT_U8 : AFMT_S16_LE;
//
//    // number of bits per sample
//    if (ioctl(audio, SNDCTL_DSP_SAMPLESIZE, &format) == -1) {
//	m_playback_error = i18n("number of bits per samples not supported");
//	return 0;
//    }
//
//    // mono/stereo selection
//    int stereo = (channels >= 2) ? 1 : 0;
//    if (ioctl(audio, SNDCTL_DSP_STEREO, &stereo) == -1) {
//	m_playback_error = i18n("stereo not supported");
//	return 0;
//    }
//
//    // playback rate
//    if (ioctl(audio, SNDCTL_DSP_SPEED, &rate) == -1) {
//	m_playback_error = i18n("playback rate not supported");
//	return 0;
//    }
//
//    // buffer size
//    ASSERT(bufbase >= MIN_PLAYBACK_BUFFER);
//    ASSERT(bufbase <= MAX_PLAYBACK_BUFFER);
//    if (bufbase < MIN_PLAYBACK_BUFFER) bufbase = MIN_PLAYBACK_BUFFER;
//    if (bufbase > MAX_PLAYBACK_BUFFER) bufbase = MAX_PLAYBACK_BUFFER;
//    if (ioctl(audio, SNDCTL_DSP_SETFRAGMENT, &bufbase) == -1) {
//	m_playback_error = i18n("unusable buffer size");
//	return 0;
//    }
//
//    // return the buffer size in bytes
//    int size;
//    ioctl(audio, SNDCTL_DSP_GETBLKSIZE, &size);
//    return size;
//}

////***************************************************************************
//void SignalManager::playback(int /*device*/, playback_param_t &/*param*/,
//                             unsigned char */*buffer*/, unsigned int /*bufsize*/,
//                             unsigned int /*start*/, bool /*loop*/)
//{
//    ASSERT(buffer);
//    ASSERT(bufsize);
//    ASSERT(param.channels);
//    if (!buffer || !bufsize || !param.channels) return;
//
//    unsigned int i;
//    unsigned int active_channels = 0;
//    unsigned int in_channels = m_channels;
//    unsigned int out_channels = param.channels;
//    unsigned int active_channel[in_channels]; // list of active channels
//
//    // get the number of channels with enabled playback
//    for (i=0; i < in_channels; i++) {
//	if (!signal.at(i)) continue;
//	// ### TODO: use state of play widget instead of "enabled" ###
//	if (!signal.at(i)->isSelected()) continue;
//
//	active_channel[active_channels++] = i;
//    }
//
//    // abort if no channels -> nothing to do
//    if (!active_channels) {
//	debug("SignalManager::playback(): no active channel, nothing to do");
//	msg[stopprocess] = true;
//    }
//
//    // set up the matrix for channel mixing
//    int matrix[active_channels][out_channels];
//    unsigned int x, y;
//    for (y=0; y < out_channels; y++) {
//	unsigned int m1, m2;
//	m1 = y * active_channels;
//	m2 = (y+1) * active_channels;
//	
//	for (x=0; x < active_channels; x++) {
//	    unsigned int n1, n2;
//	    n1 = x * out_channels;
//	    n2 = (x+1) * out_channels;
//
//	    // get the common area of [n1..n2] and [m1..m2]
//	    unsigned int left = max(n1, m1);
//	    unsigned int right = min(n2, m2);
//
//	    matrix[x][y] = (right > left) ? (right-left) : 0;
//	}
//    }
//
//    // loop until process is stopped
//    // or run once if not in loop mode
//    unsigned int pointer = start;
//    unsigned int last = rmarker;
//    int samples[active_channels];
//
//    if (lmarker == rmarker) last = getLength()-1;
//    m_spx_playback_pos.enqueue(pointer);
//    do {
//
//	while ((pointer <= last) && !msg[stopprocess]) {
//	
//	    // fill the buffer with audio data
//	    unsigned int cnt;
//	    for (cnt = 0; (cnt < bufsize) && (pointer <= last); pointer++) {
//                unsigned int channel;
//
//		for (y=0; y < out_channels; y++) {
//		    double s = 0;
//		    for (x=0; x < active_channels; x++) {
//			s += signal.at(
//				active_channel[x])->getSingleSample(
//				pointer) * matrix[x][y];
//		    }
//		    samples[y] = (int)(s / active_channels);
//		}
//
//		for (channel=0; channel < out_channels; channel++) {
//		    int sample = samples[channel];
//		
//		    switch (param.bits_per_sample) {
//			case 8:
//			    sample += 1 << 23;
//			    buffer[cnt++] = sample >> 16;
//			    break;
//			case 16:
//			    sample += 1 << 23;
//			    buffer[cnt++] = sample >> 8;
//			    buffer[cnt++] = (sample >> 16) + 128;
//			    break;
//			case 24:
//			    // play in 32 bit format
//			    buffer[cnt++] = 0x00;
//			    buffer[cnt++] = sample & 0x0FF;
//			    buffer[cnt++] = sample >> 8;
//			    buffer[cnt++] = (sample >> 16) + 128;
//
//			    break;
//			default:
//			    // invalid bits per sample
//			    msg[stopprocess] = true;
//			    pointer = last;
//			    cnt = 0;
//			    break;
//		    }
//		}
//	    }
//
//	    // write buffer to the playback device
//	    write(device, buffer, cnt);
//	    m_spx_playback_pos.enqueue(pointer);
//	}
//	
//	// maybe we loop. in this case the playback starts
//	// again from the left marker
//	if (loop && !msg[stopprocess]) pointer = lmarker;
//
//    } while (loop && !msg[stopprocess]);
//
//    // playback is done
//    m_spx_playback_done.AsyncHandler();
//}

//***************************************************************************
//***************************************************************************
