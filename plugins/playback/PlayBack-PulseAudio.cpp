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

#ifdef HAVE_PULSEAUDIO_SUPPORT
#include "config.h"

#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <QApplication>
#include <QCursor>
#include <QFileInfo>
#include <QLatin1Char>
#include <QLocale>
#include <QString>
#include <QtGlobal>

#include <KLocalizedString>
#include <KUser>

#include "libkwave/FileInfo.h"
#include "libkwave/SampleEncoderLinear.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/memcpy.h"

#include "PlayBack-PulseAudio.h"

/**
 * timeout for the device scan [ms]
 * @see scanDevices()
 */
#define TIMEOUT_WAIT_DEVICE_SCAN 10000

/**
 * timeout to wait for the connection to the server [ms]
 * @see connectToServer()
 */
#define TIMEOUT_CONNECT_TO_SERVER 20000

/**
 * timeout to wait for playback [ms]
 * @see open()
 */
#define TIMEOUT_CONNECT_PLAYBACK 10000

/**
 * minimum timeout for flush() [ms]
 * @see flush()
 */
#define TIMEOUT_MIN_FLUSH 1000

/**
 * minimum timeout for drain() [ms]
 * @see close()
 */
#define TIMEOUT_MIN_DRAIN 3000

//***************************************************************************
Kwave::PlayBackPulseAudio::PlayBackPulseAudio(const Kwave::FileInfo &info)
    :Kwave::PlayBackDevice(), m_mainloop_thread(this, QVariant()),
     m_mainloop_lock(), m_mainloop_signal(), m_info(info), m_rate(0),
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
	    m_mainloop_signal.wakeAll();
	    break;
	case PA_CONTEXT_TERMINATED:
	    qWarning("PlayBackPulseAudio: PA_CONTEXT_TERMINATED");
	    m_mainloop_signal.wakeAll();
	    break;
	case PA_CONTEXT_FAILED:
	    qWarning("PlayBackPulseAudio: PA_CONTEXT_FAILED");
	    m_mainloop_signal.wakeAll();
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
	m_mainloop_signal.wakeAll();
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
	    m_mainloop_signal.wakeAll();
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

    m_mainloop_signal.wakeAll();
}

//***************************************************************************
void Kwave::PlayBackPulseAudio::notifySuccess(pa_stream* stream, int success)
{
//     qDebug("PlayBackPulseAudio::notifySuccess(stream=%p, success=%d)",
// 	   static_cast<void *>(stream), success);
    Q_UNUSED(success);

    Q_ASSERT(stream);
    Q_ASSERT(stream == m_pa_stream);
    if (!stream || (stream != m_pa_stream)) return;

    m_mainloop_signal.wakeAll();
}

//***************************************************************************
static int poll_func(struct pollfd *ufds, unsigned long nfds,
                     int timeout, void *userdata)
{
    Kwave::PlayBackPulseAudio *dev =
	static_cast<Kwave::PlayBackPulseAudio *>(userdata);
    Q_ASSERT(dev);
    if (!dev) return -1;

    return dev->mainloopPoll(ufds, nfds, timeout);
}

//***************************************************************************
int Kwave::PlayBackPulseAudio::mainloopPoll(struct pollfd *ufds,
                                            unsigned long int nfds,
                                            int timeout)
{
    m_mainloop_lock.unlock();
    int retval = poll(ufds, nfds, timeout);
    m_mainloop_lock.lock();

    return retval;
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
                     UTF8(QLocale::system().name()));
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_NAME,
                     UTF8(qApp->applicationName()));
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_ICON_NAME,
                     "kwave");
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_PROCESS_BINARY,
                     "kwave");
    pa_proplist_setf(m_pa_proplist, PA_PROP_APPLICATION_PROCESS_ID,
                    "%ld", static_cast<long int>(qApp->applicationPid()));
    KUser user;
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_PROCESS_USER,
                     UTF8(user.loginName()));
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_VERSION,
                     UTF8(qApp->applicationVersion()));

    pa_proplist_sets(m_pa_proplist, PA_PROP_MEDIA_ROLE, "production");

    // ignore SIGPIPE in this context
#ifdef HAVE_SIGNAL_H
    signal(SIGPIPE, SIG_IGN);
#endif

    m_pa_mainloop = pa_mainloop_new();
    Q_ASSERT(m_pa_mainloop);
    pa_mainloop_set_poll_func(m_pa_mainloop, poll_func, this);

    m_pa_context = pa_context_new_with_proplist(
	pa_mainloop_get_api(m_pa_mainloop),
	"Kwave",
	m_pa_proplist
    );

    // set the callback for getting informed about the context state
    pa_context_set_state_callback(m_pa_context, pa_context_notify_cb, this);

    // connect to the pulse audio server server
    bool failed = false;
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
	failed = true;
    }

    if (!failed) {
	m_mainloop_lock.lock();
	m_mainloop_thread.start();

	// wait until the context state is either connected or failed
	failed = true;
	if ( m_mainloop_signal.wait(&m_mainloop_lock,
	                            TIMEOUT_CONNECT_TO_SERVER) )
	{
	    if (pa_context_get_state(m_pa_context) == PA_CONTEXT_READY) {
		qDebug("PlayBackPulseAudio: context is ready :-)");
		failed = false;
	    }
	}
	m_mainloop_lock.unlock();

	if (failed) {
	    qWarning("PlayBackPulseAudio: context FAILED (%s):-(",
	             pa_strerror(pa_context_errno(m_pa_context)));
	}
    }

    // if the connection failed, clean up...
    if (failed) {
	disconnectFromServer();
    }

    QApplication::restoreOverrideCursor();

    return !failed;
}

//***************************************************************************
void Kwave::PlayBackPulseAudio::disconnectFromServer()
{
    // stop the main loop
    m_mainloop_thread.cancel();
    if (m_pa_mainloop) {
	m_mainloop_lock.lock();
	pa_mainloop_quit(m_pa_mainloop, 0);
	m_mainloop_lock.unlock();
    }
    m_mainloop_thread.stop();

    // disconnect the pulse context
    if (m_pa_context) {
	pa_context_disconnect(m_pa_context);
	pa_context_unref(m_pa_context);
	m_pa_context  = 0;
    }

    // stop and free the main loop
    if (m_pa_mainloop) {
	pa_mainloop_free(m_pa_mainloop);
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

    m_rate = rate;

    if (channels > 255)
	return i18n("%1 channels are not supported, maximum is 255").arg(
	    channels);

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
    sample_spec.channels = static_cast<uint8_t>(channels);
    sample_spec.rate     = static_cast<uint32_t>(m_rate);

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
    m_mainloop_lock.lock();

    // create a new stream
    m_pa_stream = pa_stream_new_with_proplist(
	m_pa_context,
	name.toUtf8().data(),
	&sample_spec,
	0 /* const pa_channel_map *map */,
	_proplist);
    pa_proplist_free(_proplist);

    if (!m_pa_stream) {
	m_mainloop_lock.unlock();
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
	m_mainloop_signal.wait(&m_mainloop_lock, TIMEOUT_CONNECT_PLAYBACK);
	if (pa_stream_get_state(m_pa_stream) != PA_STREAM_READY)
	    result = -1;
    }
    m_mainloop_lock.unlock();

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
	m_mainloop_lock.lock();

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

	m_mainloop_lock.unlock();

	if (result < 0) {
	    qWarning("PlayBackPulseAudio: pa_stream_begin_write failed");
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
	         Kwave::toUint(m_buffer_used),
	         Kwave::toUint(m_buffer_size));
	m_buffer_used = 0;
	return -EIO;
    }

    // copy the samples
    MEMCPY(reinterpret_cast<quint8 *>(m_buffer) + m_buffer_used,
	   samples.constData(), bytes);
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
    int ms = (!qFuzzyIsNull(m_rate)) ?
	Kwave::toInt((samples_per_buffer * 1000.0) / m_rate) : 0;
    int timeout = (ms + 1) * 16;
    if (timeout < TIMEOUT_MIN_FLUSH)
	timeout = TIMEOUT_MIN_FLUSH;

    // write out the buffer allocated before in "write"
    int result = 0;

    while (m_buffer_used) {
	size_t len;

	m_mainloop_lock.lock();
	while (!(len = pa_stream_writable_size(m_pa_stream))) {
	    if (!PA_CONTEXT_IS_GOOD(pa_context_get_state(m_pa_context)) ||
		!PA_STREAM_IS_GOOD(pa_stream_get_state(m_pa_stream)) ||
		(static_cast<ssize_t>(len) == -1) )
	    {
		qWarning("PlayBackPulseAudio::flush(): bad stream state");
		result = -1;
		break;
	    }
	    if (!m_mainloop_signal.wait(&m_mainloop_lock, timeout)) {
		qWarning("PlayBackPulseAudio::flush(): timed out after %u ms",
		         timeout);
		result = -1;
		break;
	    }
        }
	m_mainloop_lock.unlock();
	if (result < 0) break;

	if (len > m_buffer_used) len = m_buffer_used;

// 	qDebug("PlayBackPulseAudio::flush(): writing %u bytes...", len);
	m_mainloop_lock.lock();
	result = pa_stream_write(
		m_pa_stream,
		m_buffer,
		len,
		0,
		0,
		PA_SEEK_RELATIVE
	);
	m_mainloop_lock.unlock();

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
void Kwave::PlayBackPulseAudio::run_wrapper(const QVariant &params)
{
    Q_UNUSED(params);
    m_mainloop_lock.lock();
    pa_mainloop_run(m_pa_mainloop, 0);
    m_mainloop_lock.unlock();
}

//***************************************************************************
int Kwave::PlayBackPulseAudio::close()
{
    // set hourglass cursor, we are waiting...
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (m_buffer_used) flush();

    if (m_pa_mainloop && m_pa_stream) {

	pa_operation *op = 0;
	m_mainloop_lock.lock();
	op = pa_stream_drain(m_pa_stream, pa_stream_success_cb, this);
	Q_ASSERT(op);
	if (!op) qWarning("pa_stream_drain() failed: '%s'", pa_strerror(
	    pa_context_errno(m_pa_context)));

	// calculate a reasonable time for the timeout (16 buffers)
	int samples_per_buffer = (m_buffer_size / m_bytes_per_sample);
	int ms = (!qFuzzyIsNull(m_rate)) ?
	    Kwave::toInt((samples_per_buffer * 1000.0) / m_rate) : 0;
	int timeout = (ms + 1) * 4;
	if (timeout < TIMEOUT_MIN_DRAIN)
	    timeout = TIMEOUT_MIN_DRAIN;

	qDebug("PlayBackPulseAudio::flush(): waiting for drain to finish...");
	while (op && (pa_operation_get_state(op) != PA_OPERATION_DONE)) {
	    if (!PA_CONTEXT_IS_GOOD(pa_context_get_state(m_pa_context)) ||
		!PA_STREAM_IS_GOOD(pa_stream_get_state(m_pa_stream))) {
		qWarning("PlayBackPulseAudio::close(): bad stream state");
		break;
	    }
	    if (!m_mainloop_signal.wait(&m_mainloop_lock, timeout)) {
		qWarning("PlayBackPulseAudio::flush(): timed out after %u ms",
		         timeout);
		break;
	    }
	}
	m_mainloop_lock.unlock();

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
    m_mainloop_lock.lock();
    m_device_list.clear();
    pa_operation *op_sink_info = pa_context_get_sink_info_list(
	m_pa_context,
	pa_sink_info_cb,
	this
    );
    if (op_sink_info) {
	// set hourglass cursor, we have a long timeout...
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	m_mainloop_signal.wait(&m_mainloop_lock, TIMEOUT_WAIT_DEVICE_SCAN);
	QApplication::restoreOverrideCursor();
    }

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
	foreach (QString sn, m_device_list.keys()) {
	    if (sn == sink) continue;
	    if ((m_device_list[sn].m_description == description) &&
		(m_device_list[sn].m_driver      == driver))
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
    m_mainloop_lock.unlock();
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
QList<unsigned int> Kwave::PlayBackPulseAudio::supportedBits
(
    const QString &device
)
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
