/***************************************************************************
       PlayBack-PulseAudio.cpp  -  playback device for PulseAudio
			     -------------------
    begin                : Tue Sep 29 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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
#ifdef HAVE_PULSEAUDIO_SUPPORT

#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <signal.h>

#include <QtGui/QApplication>
#include <QtGui/QCursor>
#include <QtCore/QFileInfo>
#include <QtCore/QLatin1Char>
#include <QtCore/QLocale>
#include <QtCore/QString>
#include <QtCore/QtGlobal>
#include <QtCore/QTime>

#include <klocale.h>
#include <kuser.h>

#include "libkwave/FileInfo.h"
#include "libkwave/memcpy.h"
#include "libkwave/SampleEncoderLinear.h"
#include "libkwave/String.h"

#include "PlayBack-PulseAudio.h"

//***************************************************************************
Kwave::PlayBackPulseAudio::PlayBackPulseAudio(const Kwave::FileInfo &info)
    :Kwave::PlayBackDevice(), m_info(info), m_rate(0), m_channels(0),
     m_bytes_per_sample(0), m_buffer(0), m_buffer_size(0), m_buffer_used(0),
     m_bufbase(10), m_pa_proplist(0), m_pa_mainloop(0), m_pa_context(0),
     m_pa_stream(0), m_device_list()
{
}

//***************************************************************************
Kwave::PlayBackPulseAudio::~PlayBackPulseAudio()
{
    close();
}

//***************************************************************************
void Kwave::PlayBackPulseAudio::pa_context_notify_cb(pa_context *c, void *data)
{
    Kwave::PlayBackPulseAudio *playback_plugin =
	reinterpret_cast<Kwave::PlayBackPulseAudio *>(data);
    Q_ASSERT(playback_plugin);
    if (playback_plugin) playback_plugin->notifyContext(c);
}

//***************************************************************************
void Kwave::PlayBackPulseAudio::pa_sink_info_cb(pa_context *c,
                                                const pa_sink_info *info,
                                                int eol, void *userdata)
{
    Kwave::PlayBackPulseAudio *playback_plugin =
	reinterpret_cast<Kwave::PlayBackPulseAudio *>(userdata);
    Q_ASSERT(playback_plugin);
    if (playback_plugin) playback_plugin->notifySinkInfo(c, info, eol);
}

//***************************************************************************
void Kwave::PlayBackPulseAudio::pa_stream_state_cb(pa_stream *p, void *userdata)
{
    Kwave::PlayBackPulseAudio *playback_plugin =
	reinterpret_cast<Kwave::PlayBackPulseAudio *>(userdata);
    Q_ASSERT(playback_plugin);
    if (playback_plugin) playback_plugin->notifyStreamState(p);
}

//***************************************************************************
void Kwave::PlayBackPulseAudio::pa_write_cb(pa_stream *p, size_t nbytes,
                                            void *userdata)
{
    Kwave::PlayBackPulseAudio *playback_plugin =
	reinterpret_cast<Kwave::PlayBackPulseAudio *>(userdata);
    Q_ASSERT(playback_plugin);
    if (playback_plugin) playback_plugin->notifyWrite(p, nbytes);
}

//***************************************************************************
void Kwave::PlayBackPulseAudio::pa_stream_success_cb(pa_stream *s,
                                                     int success,
                                                     void *userdata)
{
    Kwave::PlayBackPulseAudio *playback_plugin =
	reinterpret_cast<Kwave::PlayBackPulseAudio *>(userdata);
    Q_ASSERT(playback_plugin);
    if (playback_plugin) playback_plugin->notifySuccess(s, success);
}

//***************************************************************************
void Kwave::PlayBackPulseAudio::notifyContext(pa_context *c)
{
    Q_ASSERT(c == m_pa_context);
    switch (pa_context_get_state(c))
    {
	case PA_CONTEXT_UNCONNECTED:
// 	    qDebug("PlayBackPulseAudio: PA_CONTEXT_UNCONNECTED!?");
	    break;
	case PA_CONTEXT_CONNECTING:
// 	    qDebug("PlayBackPulseAudio: PA_CONTEXT_CONNECTING...");
	    break;
	case PA_CONTEXT_AUTHORIZING:
// 	    qDebug("PlayBackPulseAudio: PA_CONTEXT_AUTHORIZING...");
	    break;
	case PA_CONTEXT_SETTING_NAME:
// 	    qDebug("PlayBackPulseAudio: PA_CONTEXT_SETTING_NAME...");
	    break;
	case PA_CONTEXT_READY:
// 	    qDebug("PlayBackPulseAudio: PA_CONTEXT_READY.");
	    pa_threaded_mainloop_signal(m_pa_mainloop, 0);
	    break;
	case PA_CONTEXT_TERMINATED:
	    qWarning("PlayBackPulseAudio: PA_CONTEXT_TERMINATED");
	    pa_threaded_mainloop_signal(m_pa_mainloop, 0);
	    break;
	case PA_CONTEXT_FAILED:
	    qWarning("PlayBackPulseAudio: PA_CONTEXT_FAILED");
	    pa_threaded_mainloop_signal(m_pa_mainloop, 0);
	    break;
    }
}

//***************************************************************************
void Kwave::PlayBackPulseAudio::notifySinkInfo(pa_context *c,
                                               const pa_sink_info *info,
                                               int eol)
{
    Q_UNUSED(c);
    Q_ASSERT(c == m_pa_context);
    if (eol == 0) {
#if 0
	qDebug("PlayBackPulseAudio: [%d] sink='%s' (%s) driver='%s'"\
	       "card=%d, ports=%d",
	       info->index,
	       info->name,
	       info->description,
	       info->driver,
	       info->card,
	       info->n_ports
	);
	for (int p = 0; p < info->n_ports; p++) {
	    qDebug("                     [%2d] - '%s' (%s), prio=%d%s",
		   p,
		   info->ports[p]->name,
		   info->ports[p]->description,
		   info->ports[p]->priority,
		   (info->ports[p] == info->active_port) ? " <*>" : ""
	    );
	}
#endif
	sink_info_t i;
	i.m_name        = QString::fromUtf8(info->name);
	i.m_description = QString::fromUtf8(info->description);
	i.m_driver      = QString::fromUtf8(info->driver);
	i.m_card        = info->card;
	i.m_sample_spec = info->sample_spec;

	QString name    = QString::number(m_device_list.count());
	m_device_list[name] = i;

    } else {
	pa_threaded_mainloop_signal(m_pa_mainloop, 0);
    }
}

//***************************************************************************
void Kwave::PlayBackPulseAudio::notifyStreamState(pa_stream* stream)
{
    Q_ASSERT(stream);
    if (!stream || (stream != m_pa_stream)) return;

    pa_stream_state_t state = pa_stream_get_state(stream);
//     switch (state) {
// 	case PA_STREAM_UNCONNECTED:
// 	    qDebug("    -> UNCONNECTED"); break;
// 	case PA_STREAM_CREATING:
// 	    qDebug("    -> CREATING"); break;
// 	case PA_STREAM_READY:
// 	    qDebug("    -> READY"); break;
// 	case PA_STREAM_FAILED:
// 	    qDebug("    -> FAILED"); break;
// 	case PA_STREAM_TERMINATED:
// 	    qDebug("    -> TERMINATED"); break;
// 	default:
// 	    Q_ASSERT(0 && "?");
// 	    qDebug("    -> ???"); break;
//     }
    switch (state) {
	case PA_STREAM_UNCONNECTED:
	case PA_STREAM_CREATING:
	    break;
	case PA_STREAM_READY:
	case PA_STREAM_FAILED:
	case PA_STREAM_TERMINATED:
	    pa_threaded_mainloop_signal(m_pa_mainloop, 0);
	    break;
    }
}

//***************************************************************************
void Kwave::PlayBackPulseAudio::notifyWrite(pa_stream *stream, size_t nbytes)
{
//     qDebug("PlayBackPulseAudio::notifyWrite(stream=%p, nbytes=%u)",
// 	   static_cast<void *>(stream), nbytes);
    Q_UNUSED(nbytes);
    Q_ASSERT(stream);
    Q_ASSERT(stream == m_pa_stream);
    if (!stream || (stream != m_pa_stream)) return;

    pa_threaded_mainloop_signal(m_pa_mainloop, 0);
}

//***************************************************************************
void Kwave::PlayBackPulseAudio::notifySuccess(pa_stream* stream, int success)
{
    qDebug("PlayBackPulseAudio::notifySuccess(stream=%p, success=%d)",
	   static_cast<void *>(stream), success);
    Q_ASSERT(stream);
    Q_ASSERT(stream == m_pa_stream);
    if (!stream || (stream != m_pa_stream)) return;

    pa_threaded_mainloop_signal(m_pa_mainloop, 0);
}

//***************************************************************************
bool Kwave::PlayBackPulseAudio::connectToServer()
{
    if (m_pa_context) return true; // already connected

    // set hourglass cursor, we are waiting...
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // create a property list for this application
    m_pa_proplist = pa_proplist_new();
    Q_ASSERT(m_pa_proplist);

    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_LANGUAGE,
                     __(QLocale::system().name()));
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_NAME,
                     __(qApp->applicationName()));
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_ICON_NAME,
                     "kwave");
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_PROCESS_BINARY,
                     "kwave");
    pa_proplist_setf(m_pa_proplist, PA_PROP_APPLICATION_PROCESS_ID,
                    "%ld", static_cast<long int>(qApp->applicationPid()));
    KUser user;
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_PROCESS_USER,
                     __(user.loginName()));
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_VERSION,
                     __(qApp->applicationVersion()));

    pa_proplist_sets(m_pa_proplist, PA_PROP_MEDIA_ROLE, "production");

    // ignore SIGPIPE in this context
#ifdef HAVE_SIGNAL_H
    signal(SIGPIPE, SIG_IGN);
#endif

    m_pa_mainloop = pa_threaded_mainloop_new();
    Q_ASSERT(m_pa_mainloop);

    m_pa_context = pa_context_new_with_proplist(
	pa_threaded_mainloop_get_api(m_pa_mainloop),
	"Kwave",
	m_pa_proplist
    );

    // set the callback for getting informed about the context state
    pa_context_set_state_callback(m_pa_context, pa_context_notify_cb, this);

    // connect to the pulse audio server server
    bool context_connected = false;
    int error = pa_context_connect(
	m_pa_context,                       // context
	0,                                  // server
	static_cast<pa_context_flags_t>(0), // flags
	0                                   // API
    );
    if (error < 0)
    {
	qWarning("PlayBackPulseAudio: pa_contect_connect failed (%s)",
	         pa_strerror(pa_context_errno(m_pa_context)));
    } else context_connected = true;

    bool mainloop_started = false;
    if (context_connected) {
	pa_threaded_mainloop_lock(m_pa_mainloop);
	error = pa_threaded_mainloop_start(m_pa_mainloop);
	if (error < 0) {
	    qWarning("PlayBackPulseAudio: pa_threaded_mainloop_start failed (%s)",
		    pa_strerror(pa_context_errno(m_pa_context)));
	} else {
	    mainloop_started = true;
	    qDebug("PlayBackPulseAudio: mainloop started");
	}
    }
    bool failed = !mainloop_started;

    // wait until the context state is either connected or failed
    if (mainloop_started) {
	pa_threaded_mainloop_wait(m_pa_mainloop);
	if (pa_context_get_state(m_pa_context) != PA_CONTEXT_READY) {
	    qWarning("PlayBackPulseAudio: context FAILED (%s):-(",
		pa_strerror(pa_context_errno(m_pa_context)));
	    failed = true;
	} else {
	    qDebug("PlayBackPulseAudio: context is ready :-)");
	}
    }
    if (context_connected) pa_threaded_mainloop_unlock(m_pa_mainloop);

    // if the connection failed, clean up...
    if (failed) {
	// release the property list
	pa_proplist_free(m_pa_proplist);

	// disconnect the pulse context
	if (context_connected) pa_context_disconnect(m_pa_context);
	pa_context_unref(m_pa_context);

	// stop and free the main loop
	if (mainloop_started) pa_threaded_mainloop_stop(m_pa_mainloop);
	pa_threaded_mainloop_free(m_pa_mainloop);
	qDebug("PlayBackPulseAudio: mainloop freed");

	m_pa_proplist = 0;
	m_pa_context  = 0;
	m_pa_mainloop = 0;
    }

    QApplication::restoreOverrideCursor();

    return !failed;
}

//***************************************************************************
void Kwave::PlayBackPulseAudio::disconnectFromServer()
{
    // stop the main loop
    if (m_pa_mainloop) pa_threaded_mainloop_stop(m_pa_mainloop);

    // disconnect the pulse context
    if (m_pa_context) {
	pa_context_disconnect(m_pa_context);
	pa_context_unref(m_pa_context);
	m_pa_context  = 0;
    }

    // stop and free the main loop
    if (m_pa_mainloop) {
	pa_threaded_mainloop_free(m_pa_mainloop);
	m_pa_mainloop = 0;
	qDebug("PlayBackPulseAudio: mainloop freed");
    }

    // release the property list
    if (m_pa_proplist) {
	pa_proplist_free(m_pa_proplist);
	m_pa_proplist = 0;
    }

}

//***************************************************************************
QString Kwave::PlayBackPulseAudio::open(const QString &device, double rate,
                                        unsigned int channels,
                                        unsigned int bits,
                                        unsigned int bufbase)
{
    #define SET_PROPERTY(__property__,__info__)              \
	if (m_info.contains(__info__))                       \
	    pa_proplist_sets(_proplist, __property__,        \
	    m_info.get(__info__).toString().toUtf8().data())

    qDebug("PlayBackPulseAudio::open(device=%s,rate=%0.1f,channels=%u,"\
	"bits=%u, bufbase=%u)",
	DBG(device.split(_("|")).at(0)), rate, channels,
	bits, bufbase);

    // close the previous device
    if (m_pa_stream) close();

    // make sure that we are connected to the sound server
    if (!connectToServer()) {
	return i18n("Connecting to the PulseAudio server failed.");
    }

    if (!m_device_list.contains(device)) scanDevices();
    if (!m_device_list.contains(device)) {
	return i18n(
	    "The PulseAudio device '%1' is unknown or no longer connected",
	    device.section(QLatin1Char('|'), 0, 0));
    }
    QString pa_device = m_device_list[device].m_name;

    // determine the buffer size
    m_bytes_per_sample = sizeof(sample_t) * channels;
    m_buffer_size      = 0;
    m_buffer           = 0;
    m_bufbase          = bufbase;

    // build a property list for the stream
    pa_proplist *_proplist = pa_proplist_copy(m_pa_proplist);
    Q_ASSERT(_proplist);
    SET_PROPERTY(PA_PROP_MEDIA_TITLE,     Kwave::INF_NAME);
    SET_PROPERTY(PA_PROP_MEDIA_ARTIST,    Kwave::INF_AUTHOR);
#ifdef PA_PROP_MEDIA_COPYRIGHT
    SET_PROPERTY(PA_PROP_MEDIA_COPYRIGHT, Kwave::INF_COPYRIGHT);
#endif
#ifdef PA_PROP_MEDIA_SOFTWARE
    SET_PROPERTY(PA_PROP_MEDIA_SOFTWARE,  Kwave::INF_SOFTWARE);
#endif
//  SET_PROPERTY(PA_PROP_MEDIA_LANGUAGE,  Kwave::INF_...);
    SET_PROPERTY(PA_PROP_MEDIA_FILENAME,  Kwave::INF_FILENAME);
//  SET_PROPERTY(PA_PROP_MEDIA_ICON_NAME, Kwave::INF_...);

    // use Kwave's internal sample format as output
    pa_sample_spec sample_spec;
#if (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    sample_spec.format = PA_SAMPLE_S24_32BE;
#else
    sample_spec.format = PA_SAMPLE_S24_32LE;
#endif
    sample_spec.channels = channels;
    sample_spec.rate     = static_cast<quint32>(rate);

    // use the current title / filename or fixed string as stream name
    QString name;
    if (m_info.contains(Kwave::INF_NAME)) // first choice: title
	name = m_info.get(Kwave::INF_NAME).toString();
    // fallback: filename
    if (!name.length() && m_info.contains(Kwave::INF_FILENAME))
	name = m_info.get(Kwave::INF_FILENAME).toString();
    if (!name.length()) // last resort: fixed string
	name = i18n("playback...");

    // run with mainloop locked from here on...
    pa_threaded_mainloop_lock(m_pa_mainloop);

    // create a new stream
    m_pa_stream = pa_stream_new_with_proplist(
	m_pa_context,
	name.toUtf8().data(),
	&sample_spec,
	0 /* const pa_channel_map *map */,
	_proplist);
    pa_proplist_free(_proplist);

    if (!m_pa_stream) {
	pa_threaded_mainloop_unlock(m_pa_mainloop);
	return i18n("Failed to create a PulseAudio stream (%1).",
	            _(pa_strerror(pa_context_errno(m_pa_context))));
    }
    qDebug("PlayBackPulseAudio::open(...) - stream created as %p",
           static_cast<void *>(m_pa_stream));

    // register callbacks for changes in stream state and write events
    pa_stream_set_state_callback(m_pa_stream, pa_stream_state_cb, this);
    pa_stream_set_write_callback(m_pa_stream, pa_write_cb, this);

    // set buffer attributes
    if (m_bufbase < 10) m_bufbase = 10;
    const int s = ((1 << m_bufbase) * m_bytes_per_sample) / m_bytes_per_sample;
    pa_buffer_attr attr;
    attr.fragsize  = -1;
    attr.maxlength =  s;
    attr.minreq    = -1;
    attr.prebuf    = -1;
    attr.tlength   = -1;

    // connect the stream in playback mode
    int result = pa_stream_connect_playback(
	m_pa_stream,
	pa_device.length() ? pa_device.toUtf8().data() : 0,
	&attr /* buffer attributes */,
	static_cast<pa_stream_flags_t>(
	    PA_STREAM_INTERPOLATE_TIMING |
	    PA_STREAM_AUTO_TIMING_UPDATE),
	0 /* volume */,
	0 /* sync stream */ );

    if (result >= 0) {
	pa_threaded_mainloop_wait(m_pa_mainloop);
	if (pa_stream_get_state(m_pa_stream) != PA_STREAM_READY)
	    result = -1;
    }
    pa_threaded_mainloop_unlock(m_pa_mainloop);

    if (result < 0) {
	pa_stream_unref(m_pa_stream);
	m_pa_stream = 0;
	return i18n("Failed to open a PulseAudio stream for playback (%1).",
	            _(pa_strerror(pa_context_errno(m_pa_context))));
    }

    return QString();
}

//***************************************************************************
int Kwave::PlayBackPulseAudio::write(const Kwave::SampleArray &samples)
{
    unsigned int bytes = m_bytes_per_sample;

    // abort if byte per sample is unknown
    Q_ASSERT(m_bytes_per_sample);
    Q_ASSERT(m_pa_mainloop);
    if (!m_bytes_per_sample || !m_pa_mainloop)
	return -EINVAL;

    // start with a new/empty buffer from PulseAudio
    if (!m_buffer) {
	pa_threaded_mainloop_lock(m_pa_mainloop);

	// estimate buffer size and round to whole samples
	size_t size = -1;
	m_buffer_size = (1 << m_bufbase) * m_bytes_per_sample;

	// get a buffer from PulseAudio
	int result = pa_stream_begin_write(m_pa_stream, &m_buffer, &size);
	size /= m_bytes_per_sample;
	size *= m_bytes_per_sample;

	// we don't use all of it to reduce latency, use only the
	// minimum of our configured size and pulse audio's offer
	if (size < m_buffer_size)
	    m_buffer_size = size;

	pa_threaded_mainloop_unlock(m_pa_mainloop);

	if (result < 0) {
	    qWarning("PlayBackPulseAudio::write(): pa_stream_begin_write failed");
	    return -EIO;
	}

// 	qDebug("PlayBackPulseAudio::write(): got buffer %p, size=%u bytes",
// 	       m_buffer, m_buffer_size);
    }

    // abort with out-of-memory if failed
    Q_ASSERT(m_buffer);
    if (!m_buffer || !m_buffer_size)
	return -ENOMEM;

    Q_ASSERT (m_buffer_used + bytes <= m_buffer_size);
    if (m_buffer_used + bytes > m_buffer_size) {
	qWarning("PlayBackPulseAudio::write(): buffer overflow ?! (%u/%u)",
	         static_cast<unsigned int>(m_buffer_used),
	         static_cast<unsigned int>(m_buffer_size));
	m_buffer_used = 0;
	return -EIO;
    }

    // copy the samples
    MEMCPY(reinterpret_cast<quint8 *>(m_buffer) + m_buffer_used,
	   samples.data(), bytes);
    m_buffer_used += bytes;

    // write the buffer if it is full
    if (m_buffer_used >= m_buffer_size) return flush();
    return 0;
}

//***************************************************************************
int Kwave::PlayBackPulseAudio::flush()
{
    if (!m_buffer_used || !m_pa_mainloop || !m_buffer || !m_buffer_size)
	return 0;
//     qDebug("PlayBackPulseAudio::flush(): using buffer %p (%u bytes)",
//             m_buffer, m_buffer_size);

    // calculate a reasonable time for the timeout (16 buffers)
    int samples_per_buffer = (m_buffer_size / m_bytes_per_sample);
    int ms = (samples_per_buffer * 1000) / m_info.rate();
    int timeout = (ms + 1) * 16;

    // write out the buffer allocated before in "write"
    int result = 0;

    while (m_buffer_used) {
	size_t len;

	QTime t;
	t.start();
	pa_threaded_mainloop_lock(m_pa_mainloop);
	qDebug("waiting for writable size != 0...");
	while (!(len = pa_stream_writable_size(m_pa_stream))) {
	    qDebug("len=%d",len);
	    if (!PA_CONTEXT_IS_GOOD(pa_context_get_state(m_pa_context)) ||
		!PA_STREAM_IS_GOOD(pa_stream_get_state(m_pa_stream)) ||
		(static_cast<ssize_t>(len) == -1) )
	    {
		qWarning("PlayBackPulseAudio::flush(): bad stream state");
		result = -1;
		break;
	    }
	    if (t.elapsed() >= timeout) {
		qWarning("PlayBackPulseAudio::flush(): timed out after %u ms",
		         timeout);
		result = -1;
		break;
	    }

	    pa_threaded_mainloop_wait(m_pa_mainloop);
        }
	pa_threaded_mainloop_unlock(m_pa_mainloop);
	if (result < 0) break;

	if (len > m_buffer_used) len = m_buffer_used;

	qDebug("PlayBackPulseAudio::flush(): writing %u bytes...", len);
	pa_threaded_mainloop_lock(m_pa_mainloop);
	result = pa_stream_write(
		m_pa_stream,
		m_buffer,
		len,
		0,
		0,
		PA_SEEK_RELATIVE
	);
	pa_threaded_mainloop_unlock(m_pa_mainloop);

	if (result < 0) {
	    qWarning("PlayBackPulseAudio::flush(): pa_stream_write failed");
	    return -EIO;
	}

	m_buffer       = reinterpret_cast<quint8 *>(m_buffer) + len;
	m_buffer_used -= len;
    }

//     qDebug("PlayBackPulseAudio::flush(): flush done.");

    // buffer is written out now
    m_buffer_used = 0;
    m_buffer      = 0;
    return result;
}

//***************************************************************************
int Kwave::PlayBackPulseAudio::close()
{
    // set hourglass cursor, we are waiting...
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (m_buffer_used) flush();

    if (m_pa_mainloop && m_pa_stream) {

	pa_operation *op = 0;
	pa_threaded_mainloop_lock(m_pa_mainloop);
	op = pa_stream_drain(m_pa_stream, pa_stream_success_cb, this);
	Q_ASSERT(op);
	if (!op) qWarning("pa_stream_drain() failed: '%s'", pa_strerror(
	    pa_context_errno(m_pa_context)));

	// calculate a reasonable time for the timeout (16 buffers)
	int samples_per_buffer = (m_buffer_size / m_bytes_per_sample);
	int ms = (samples_per_buffer * 1000) / m_info.rate();
	int timeout = (ms + 1) * 4;
	if (timeout < 1000) timeout = 1000;

	qDebug("PlayBackPulseAudio::flush(): waiting for drain to finish...");
	QTime t;
	t.start();
	while (op && (pa_operation_get_state(op) != PA_OPERATION_DONE)) {
	    if (!PA_CONTEXT_IS_GOOD(pa_context_get_state(m_pa_context)) ||
		!PA_STREAM_IS_GOOD(pa_stream_get_state(m_pa_stream))) {
		qWarning("PlayBackPulseAudio::close(): bad stream state");
		break;
	    }
	    if (t.elapsed() >= timeout) {
		qWarning("PlayBackPulseAudio::flush(): timed out after %u ms",
		         timeout);
		break;
	    }
	    pa_threaded_mainloop_wait(m_pa_mainloop);
	}
	pa_threaded_mainloop_unlock(m_pa_mainloop);

	if (m_pa_stream) {
	    pa_stream_disconnect(m_pa_stream);
	    pa_stream_unref(m_pa_stream);
	    m_pa_stream = 0;
	}
    }

    disconnectFromServer();
    m_device_list.clear();

    QApplication::restoreOverrideCursor();
    return 0;
}

//***************************************************************************
void Kwave::PlayBackPulseAudio::scanDevices()
{
    if (!m_pa_context) connectToServer();
    if (!m_pa_context) return;

    // fetch the device list from the PulseAudio server
    pa_threaded_mainloop_lock(m_pa_mainloop);
    m_device_list.clear();
    pa_operation *op_sink_info = pa_context_get_sink_info_list(
	m_pa_context,
	pa_sink_info_cb,
	this
    );
    if (op_sink_info) pa_threaded_mainloop_wait(m_pa_mainloop);

    // create a list with final names
//     qDebug("----------------------------------------");
    QMap<QString, sink_info_t> list;

    // first entry == default device
    sink_info_t i;
    pa_sample_spec s;
    s.format   = PA_SAMPLE_INVALID;
    s.rate     = 0;
    s.channels = 0;
    i.m_name        = QString();
    i.m_description = _("(server default)");
    i.m_driver      = QString();
    i.m_card        = -1;
    i.m_sample_spec = s;
    list[i18n("(Use server default)") + _("|sound_note")] = i;

    foreach (QString sink, m_device_list.keys()) {
	QString name        = m_device_list[sink].m_name;
	QString description = m_device_list[sink].m_description;
	QString driver      = m_device_list[sink].m_driver;

	// if the name is not unique, add the internal sink name
	int unique = true;
	foreach (QString s, m_device_list.keys()) {
	    if (s == sink) continue;
	    if ((m_device_list[s].m_description == description) &&
		(m_device_list[s].m_driver      == driver))
	    {
		unique = false;
		break;
	    }
	}
	if (!unique) description += _(" [") + name + _("]");

	// mangle the driver name, e.g.
	// "module-alsa-sink.c" -> "alsa sink"
	QFileInfo f(driver);
	driver = f.baseName();
	driver.replace(_("-"), _(" "));
	driver.replace(_("_"), _(" "));
	if (driver.toLower().startsWith(_("module ")))
	    driver.remove(0, 7);
	description.prepend(driver + _("|sound_card||"));

	// add the leaf node
	if (m_device_list[sink].m_card != PA_INVALID_INDEX)
	    description.append(_("|sound_device"));
	else
	    description.append(_("|sound_note"));

// 	qDebug("supported device: '%s'", DBG(description));
	list.insert(description, m_device_list[sink]);
    }
//     qDebug("----------------------------------------");

    m_device_list = list;
    pa_threaded_mainloop_unlock(m_pa_mainloop);
}

//***************************************************************************
QStringList Kwave::PlayBackPulseAudio::supportedDevices()
{
    QStringList list;

    // re-validate the list if necessary
    scanDevices();

    if (!m_pa_mainloop || !m_pa_context) return list;

    list = m_device_list.keys();
    if (!list.isEmpty()) list.prepend(_("#TREE#"));

    return list;
}

//***************************************************************************
QString Kwave::PlayBackPulseAudio::fileFilter()
{
    return _("");
}

//***************************************************************************
QList<unsigned int> Kwave::PlayBackPulseAudio::supportedBits(const QString &device)
{
    QList<unsigned int> list;

    if (m_device_list.isEmpty() || !m_device_list.contains(device))
	return list;

    list.append(pa_sample_size(&m_device_list[device].m_sample_spec) * 8);

    return list;
}

//***************************************************************************
int Kwave::PlayBackPulseAudio::detectChannels(const QString &device,
                                              unsigned int &min,
                                              unsigned int &max)
{
    min = max = 0;

    if (m_device_list.isEmpty() || !m_device_list.contains(device))
	return -1;

    min = max = m_device_list[device].m_sample_spec.channels;
    return 0;
}

#endif /* HAVE_PULSEAUDIO_SUPPORT */

//***************************************************************************
//***************************************************************************
