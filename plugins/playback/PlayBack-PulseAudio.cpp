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
#include "libkwave/memcpy.h"
#include "libkwave/SampleFormat.h"

#include "PlayBack-PulseAudio.h"
#include "SampleEncoderLinear.h"

//***************************************************************************
PlayBackPulseAudio::PlayBackPulseAudio()
    :PlayBackDevice(),
    m_rate(0),
    m_channels(0),
    m_bufbase(0),
    m_buffer(),
    m_buffer_size(0),
    m_buffer_used(0),
    m_pa_mainloop(0),
    m_pa_context(0),
    m_device_list()
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
	qWarning("PlayBackPulseAudio: sink info done");
	pa_threaded_mainloop_signal(m_pa_mainloop, 0);
    }
}

//***************************************************************************
bool PlayBackPulseAudio::connectToServer()
{
    if (m_pa_context) return true; // already connected

    // set hourglass cursor, we are waiting...
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // create a property list for this application
    pa_proplist *_pa_proplist = pa_proplist_new();
    Q_ASSERT(_pa_proplist);

    pa_proplist_sets(_pa_proplist, PA_PROP_APPLICATION_LANGUAGE,
                     QLocale::system().name ().toUtf8().data());
    pa_proplist_sets(_pa_proplist, PA_PROP_APPLICATION_NAME,
                     qApp->applicationName().toUtf8().data());
    pa_proplist_sets(_pa_proplist, PA_PROP_APPLICATION_PROCESS_BINARY,
                     "kwave");
    pa_proplist_setf(_pa_proplist, PA_PROP_APPLICATION_PROCESS_ID,
                    "%ld", static_cast<long int>(qApp->applicationPid()));
    KUser user;
    pa_proplist_sets(_pa_proplist, PA_PROP_APPLICATION_PROCESS_USER,
                     user.loginName().toUtf8().data());
    pa_proplist_sets(_pa_proplist, PA_PROP_APPLICATION_VERSION,
                     qApp->applicationVersion().toUtf8().data());

    pa_proplist_sets(_pa_proplist, PA_PROP_MEDIA_ROLE, "production");
    qDebug("PlayBackPulseAudio: created property list.");

    // ignore SIGPIPE in this context
#ifdef HAVE_SIGNAL_H
    signal(SIGPIPE, SIG_IGN);
#endif

    m_pa_mainloop = pa_threaded_mainloop_new();
    Q_ASSERT(m_pa_mainloop);

    qDebug("PlayBackPulseAudio: creating context...");
    m_pa_context = pa_context_new_with_proplist(
	pa_threaded_mainloop_get_api(m_pa_mainloop),
	"Kwave",
	_pa_proplist
    );
    qDebug("PlayBackPulseAudio: context created as %p",
	   static_cast<void *>(m_pa_context));

    // release the property list again
    pa_proplist_free(_pa_proplist);

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

    // wait until the context state is either connected or failed
    bool failed = !mainloop_started;
    if (mainloop_started) {
	pa_threaded_mainloop_wait(m_pa_mainloop);
	if (pa_context_get_state(m_pa_context) != PA_CONTEXT_READY)
	{
	    qWarning("PlayBackPulseAudio: context FAILED :-(");
	    failed = true;
	} else {
	    qDebug("PlayBackPulseAudio: context is ready :-)");
	}
    }
    if (context_connected) pa_threaded_mainloop_unlock(m_pa_mainloop);

    // if the connection failed, clean up...
    if (failed) {
	// disconnect the pulse context
	if (context_connected) pa_context_disconnect(m_pa_context);
	pa_context_unref(m_pa_context);

	// stop and free the main loop
	if (mainloop_started) pa_threaded_mainloop_stop(m_pa_mainloop);
	pa_threaded_mainloop_free(m_pa_mainloop);
	qDebug("PlayBackPulseAudio: mainloop freed");

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

    /* TODO */
//  pa_stream_disconnect(...);
//  pa_stream_unref(...);

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
}

//***************************************************************************
QString PlayBackPulseAudio::open(const QString &device, double rate,
                           unsigned int channels, unsigned int bits,
                           unsigned int bufbase)
{
    qDebug("PlayBackPulseAudio::open(device=%s,rate=%0.1f,channels=%u,"\
	"bits=%u, bufbase=%u)", device.toLocal8Bit().data(), rate, channels,
	bits, bufbase);

    // close the previous device
    /* TODO */

    // initialize the list of supported formats
    /* TODO */

    QString reason;
    if (!connectToServer()) {
	return i18n("Connection to the PulseAudio server failed.");
    }

#if 0
    /* TODO */
    int err = openStream(device, rate, channels, bits);
    if (err) {
	QString reason;
	switch (err) {
	    case ...
	    default:
		reason = i18n("Opening the device '%1' failed: %2",
	            device.section('|',0,0),
		    QString::fromLocal8Bit(snd_strerror(err)));
	}
	return reason;
    }
#endif
    return 0;
}

//***************************************************************************
int PlayBackPulseAudio::write(const Kwave::SampleArray &samples)
{
    /* TODO */
    Q_UNUSED(samples);
    return 0;
}

//***************************************************************************
int PlayBackPulseAudio::flush()
{
    /* TODO */
    return 0;
}

//***************************************************************************
int PlayBackPulseAudio::close()
{
    flush();

    disconnectFromServer();
    m_device_list.clear();

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
