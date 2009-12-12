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

#include <QApplication>
#include <QCursor>
#include <QFileInfo>
#include <QLocale>
#include <QString>
#include <QtGlobal>

#include <klocale.h>
#include <kuser.h>

#include "libkwave/CompressionType.h"
#include "libkwave/FileInfo.h"
#include "libkwave/memcpy.h"
#include "libkwave/SampleFormat.h"

#include "PlayBack-PulseAudio.h"
#include "SampleEncoderLinear.h"

//***************************************************************************
PlayBackPulseAudio::PlayBackPulseAudio(const FileInfo &info)
    :PlayBackDevice(), m_info(info), m_rate(0), m_channels(0),
     m_bytes_per_sample(0), m_buffer(0), m_buffer_size(0), m_buffer_used(0),
     m_bufbase(10), m_pa_proplist(0), m_pa_mainloop(0), m_pa_context(0),
     m_pa_stream(0), m_device_list()
{
}

//***************************************************************************
PlayBackPulseAudio::~PlayBackPulseAudio()
{
    close();
}

//***************************************************************************
void PlayBackPulseAudio::pa_context_notify_cb(pa_context *c, void *data)
{
    PlayBackPulseAudio *playback_plugin =
	reinterpret_cast<PlayBackPulseAudio *>(data);
    Q_ASSERT(playback_plugin);
    if (playback_plugin) playback_plugin->notifyContext(c);
}

//***************************************************************************
void PlayBackPulseAudio::pa_sink_info_cb(pa_context *c,
                                         const pa_sink_info *info,
                                         int eol, void *userdata)
{
    PlayBackPulseAudio *playback_plugin =
	reinterpret_cast<PlayBackPulseAudio *>(userdata);
    Q_ASSERT(playback_plugin);
    if (playback_plugin) playback_plugin->notifySinkInfo(c, info, eol);
}

//***************************************************************************
void PlayBackPulseAudio::pa_stream_state_cb(pa_stream *p, void *userdata)
{
    PlayBackPulseAudio *playback_plugin =
	reinterpret_cast<PlayBackPulseAudio *>(userdata);
    Q_ASSERT(playback_plugin);
    if (playback_plugin) playback_plugin->notifyStreamState(p);
}

//***************************************************************************
void PlayBackPulseAudio::pa_write_cb(pa_stream *p, size_t nbytes,
                                     void *userdata)
{
    PlayBackPulseAudio *playback_plugin =
	reinterpret_cast<PlayBackPulseAudio *>(userdata);
    Q_ASSERT(playback_plugin);
    if (playback_plugin) playback_plugin->notifyWrite(p, nbytes);
}

//***************************************************************************
void PlayBackPulseAudio::pa_stream_success_cb(pa_stream *s, int success,
                                              void *userdata)
{
    PlayBackPulseAudio *playback_plugin =
	reinterpret_cast<PlayBackPulseAudio *>(userdata);
    Q_ASSERT(playback_plugin);
    if (playback_plugin) playback_plugin->notifySuccess(s, success);
}

//***************************************************************************
void PlayBackPulseAudio::pa_stream_latency_cb(pa_stream *p, void *userdata)
{
    PlayBackPulseAudio *playback_plugin =
	reinterpret_cast<PlayBackPulseAudio *>(userdata);
    Q_ASSERT(playback_plugin);
    if (playback_plugin) playback_plugin->notifyLatency(p);
}

//***************************************************************************
void PlayBackPulseAudio::notifyContext(pa_context *c)
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
void PlayBackPulseAudio::notifySinkInfo(pa_context *c,
                                        const pa_sink_info *info, int eol)
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
void PlayBackPulseAudio::notifyStreamState(pa_stream* stream)
{
    Q_ASSERT(stream);
    Q_ASSERT(stream = m_pa_stream);
    if (!stream || (stream != m_pa_stream)) return;

    switch (pa_stream_get_state(stream)) {
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
void PlayBackPulseAudio::notifyWrite(pa_stream *stream, size_t nbytes)
{
    Q_UNUSED(nbytes);
    Q_ASSERT(stream);
    Q_ASSERT(stream = m_pa_stream);
    if (!stream || (stream != m_pa_stream)) return;

//     qDebug("PlayBackPulseAudio::notifyWrite(stream=%p, nbytes=%u)",
// 	   static_cast<void *>(stream), nbytes);
    pa_threaded_mainloop_signal(m_pa_mainloop, 0);
}

//***************************************************************************
void PlayBackPulseAudio::notifyLatency(pa_stream *stream)
{
    Q_ASSERT(stream);
    Q_ASSERT(stream = m_pa_stream);
    if (!stream || (stream != m_pa_stream)) return;

//     qDebug("PlayBackPulseAudio::notifyLatency(stream=%p)",
// 	   static_cast<void *>(stream));
//     pa_threaded_mainloop_signal(m_pa_mainloop, 0);
}

//***************************************************************************
void PlayBackPulseAudio::notifySuccess(pa_stream* stream, int success)
{
    Q_ASSERT(stream);
    Q_ASSERT(stream = m_pa_stream);
    if (!stream || (stream != m_pa_stream)) return;

    qDebug("PlayBackPulseAudio::notifySuccess(stream=%p, success=%d)",
	   static_cast<void *>(stream), success);
    pa_threaded_mainloop_signal(m_pa_mainloop, 0);
}

//***************************************************************************
bool PlayBackPulseAudio::connectToServer()
{
    if (m_pa_context) return true; // already connected

    // set hourglass cursor, we are waiting...
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // create a property list for this application
    m_pa_proplist = pa_proplist_new();
    Q_ASSERT(m_pa_proplist);

    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_LANGUAGE,
                     QLocale::system().name ().toUtf8().data());
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_NAME,
                     qApp->applicationName().toUtf8().data());
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_ICON_NAME,
                     "kwave");
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_PROCESS_BINARY,
                     "kwave");
    pa_proplist_setf(m_pa_proplist, PA_PROP_APPLICATION_PROCESS_ID,
                    "%ld", static_cast<long int>(qApp->applicationPid()));
    KUser user;
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_PROCESS_USER,
                     user.loginName().toUtf8().data());
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_VERSION,
                     qApp->applicationVersion().toUtf8().data());

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
void PlayBackPulseAudio::disconnectFromServer()
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
QString PlayBackPulseAudio::open(const QString &device, double rate,
                                 unsigned int channels, unsigned int bits,
                                 unsigned int bufbase)
{
    #define SET_PROPERTY(__property__,__info__)              \
	if (m_info.contains(__info__))                       \
	    pa_proplist_sets(_proplist, __property__,        \
	    m_info.get(__info__).toString().toUtf8().data())

    qDebug("PlayBackPulseAudio::open(device=%s,rate=%0.1f,channels=%u,"\
	"bits=%u, bufbase=%u)", device.toLocal8Bit().data(), rate, channels,
	bits, bufbase);

    // close the previous device
    if (m_pa_stream) close();

    // make sure that we are connected to the sound server
    if (!connectToServer()) {
	return i18n("Connection to the PulseAudio server failed.");
    }

    if (!m_device_list.contains(device)) scanDevices();
    if (!m_device_list.contains(device)) {
	return i18n(
	    "The PulseAudio device '%1' is unknown or no longer connected",
	    device.section('|',0,0).toLocal8Bit().data());
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
    SET_PROPERTY(PA_PROP_MEDIA_TITLE,     INF_NAME);
    SET_PROPERTY(PA_PROP_MEDIA_ARTIST,    INF_AUTHOR);
    SET_PROPERTY(PA_PROP_MEDIA_COPYRIGHT, INF_COPYRIGHT);
    SET_PROPERTY(PA_PROP_MEDIA_SOFTWARE,  INF_SOFTWARE);
//  SET_PROPERTY(PA_PROP_MEDIA_LANGUAGE,  INF_...);
    SET_PROPERTY(PA_PROP_MEDIA_FILENAME,  INF_FILENAME);
//  SET_PROPERTY(PA_PROP_MEDIA_ICON_NAME, INF_...);

    // use Kwave's internal sample format as output
    pa_sample_spec sample_spec;
#if (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    sample_spec.format = PA_SAMPLE_S24_32BE;
#else
    sample_spec.format = PA_SAMPLE_S24_32LE;
#endif
    sample_spec.channels = channels;
    sample_spec.rate     = static_cast<uint32_t>(rate);

    // use the current title / filename or fixed string as stream name
    QString name;
    if (m_info.contains(INF_NAME)) // first choice: title
	name = m_info.get(INF_NAME).toString().toUtf8().data();
    if (!name.length() && m_info.contains(INF_FILENAME)) // fallback: filename
	name = m_info.get(INF_FILENAME).toString().toUtf8().data();
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
	             pa_strerror(pa_context_errno(m_pa_context)));
    }
    qDebug("PlayBackPulseAudio::open(...) - stream created as %p",
	   static_cast<void *>(m_pa_stream));

    // register callbacks for changes in stream state and write events
    pa_stream_set_state_callback(m_pa_stream, pa_stream_state_cb, this);
    pa_stream_set_write_callback(m_pa_stream, pa_write_cb, this);
    pa_stream_set_latency_update_callback(m_pa_stream, pa_stream_latency_cb, this);

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
	             pa_strerror(pa_context_errno(m_pa_context)));
    }

    return 0;
}

//***************************************************************************
int PlayBackPulseAudio::write(const Kwave::SampleArray &samples)
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
// 	qDebug("PlayBackPulseAudio::write(): max buffer size=%u", size);
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
	         m_buffer_used, m_buffer_size);
	m_buffer_used = 0;
	return -EIO;
    }

    // copy the samples
    MEMCPY(reinterpret_cast<u_int8_t *>(m_buffer) + m_buffer_used,
	   samples.data(), bytes);
    m_buffer_used += bytes;

    // write the buffer if it is full
    if (m_buffer_used >= m_buffer_size) return flush();
    return 0;
}

//***************************************************************************
int PlayBackPulseAudio::flush()
{
    if (!m_buffer_used || !m_pa_mainloop || !m_buffer || !m_buffer_size)
	return 0;
//     qWarning("PlayBackPulseAudio::flush(): using buffer %p (%u bytes)",
// 	     m_buffer, m_buffer_size);

    pa_threaded_mainloop_lock(m_pa_mainloop);

    // write out the buffer allocated before in "write"
    int result = 0;
    while (m_buffer_used) {
	size_t len;

        while (!(len = pa_stream_writable_size(m_pa_stream))) {
	    if (!PA_CONTEXT_IS_GOOD(pa_context_get_state(m_pa_context)) ||
		!PA_STREAM_IS_GOOD(pa_stream_get_state(m_pa_stream))) {
		qWarning("PlayBackPulseAudio::flush(): bad stream state");
		result = -1;
		break;
	    }
            pa_threaded_mainloop_wait(m_pa_mainloop);
        }
        if (result < 0) break;

	if (len > m_buffer_used) len = m_buffer_used;

// 	qDebug("PlayBackPulseAudio::flush(): writing %u bytes...", len);
	result = pa_stream_write(
		m_pa_stream,
		m_buffer,
		len,
		0,
		0,
		PA_SEEK_RELATIVE
	);
	if (result < 0) {
	    qWarning("PlayBackPulseAudio::flush(): pa_stream_write failed");
	    pa_threaded_mainloop_unlock(m_pa_mainloop);
	    return -EIO;
	}

	m_buffer       = reinterpret_cast<u_int8_t *>(m_buffer) + len;
	m_buffer_used -= len;
    }

//     qDebug("PlayBackPulseAudio::flush(): flush done.");

    // buffer is written out now
    m_buffer_used = 0;
    m_buffer      = 0;

    pa_threaded_mainloop_unlock(m_pa_mainloop);

    return result;
}

//***************************************************************************
int PlayBackPulseAudio::close()
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

	qDebug("PlayBackPulseAudio::flush(): waiting for drain to finish...");
	while (op && (pa_operation_get_state(op) != PA_OPERATION_DONE)) {
	    if (!PA_CONTEXT_IS_GOOD(pa_context_get_state(m_pa_context)) ||
		!PA_STREAM_IS_GOOD(pa_stream_get_state(m_pa_stream))) {
		qWarning("PlayBackPulseAudio::close(): bad stream state");
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
void PlayBackPulseAudio::scanDevices()
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
    i.m_description = "(server default)";
    i.m_driver      = QString();
    i.m_card        = -1;
    i.m_sample_spec = s;
    list[i18n("(use server default)") + "|sound_note"] = i;

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
	if (!unique) description += " [" + name + "]";

	// mangle the driver name, e.g.
	// "module-alsa-sink.c" -> "alsa sink"
	QFileInfo f(driver);
	driver = f.baseName();
	driver.replace("-", " ");
	driver.replace("_", " ");
	if (driver.toLower().startsWith("module ")) driver.remove(0, 7);
	description.prepend(driver + "|sound_card||");

	// add the leaf node
	if (m_device_list[sink].m_card != PA_INVALID_INDEX)
	    description.append(QString("|sound_device"));
	else
	    description.append(QString("|sound_note"));

// 	qDebug("supported device: '%s'", description.toLocal8Bit().data());
	list.insert(description, m_device_list[sink]);
    }
//     qDebug("----------------------------------------");

    m_device_list = list;
    pa_threaded_mainloop_unlock(m_pa_mainloop);
}

//***************************************************************************
QStringList PlayBackPulseAudio::supportedDevices()
{
    QStringList list;

    // re-validate the list if necessary
    scanDevices();

    if (!m_pa_mainloop || !m_pa_context) return list;

    list = m_device_list.keys();
    if (!list.isEmpty()) list.prepend("#TREE#");

    return list;
}

//***************************************************************************
QString PlayBackPulseAudio::fileFilter()
{
    return "";
}

//***************************************************************************
QList<unsigned int> PlayBackPulseAudio::supportedBits(const QString &device)
{
    QList<unsigned int> list;

    if (m_device_list.isEmpty() || !m_device_list.contains(device))
	return list;

    list.append(pa_sample_size(&m_device_list[device].m_sample_spec) * 8);

    return list;
}

//***************************************************************************
int PlayBackPulseAudio::detectChannels(const QString &device,
                                       unsigned int &min, unsigned int &max)
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
