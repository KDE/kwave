/***************************************************************************
                    KwavePlugin.cpp  -  New Interface for Kwave plugins
                             -------------------
    begin                : Thu Jul 27 2000
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
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>
#include <sched.h>

#include <qwidget.h>

#include <kapp.h>

#include "mt/Thread.h"
#include "mt/MutexGuard.h"
#include "libgui/KwavePlugin.h"
#include "libgui/PluginContext.h"

#include "../kwave/TopWidget.h"
#include "../kwave/PluginManager.h"

//***************************************************************************
KwavePlugin::KwavePlugin(PluginContext &c)
    :m_context(c), m_thread(0)
{
    m_thread_lock.setName("KwavePlugin");
}

//***************************************************************************
KwavePlugin::~KwavePlugin()
{
    // inform our owner that we close. This allows the plugin to
    // delete itself
    close();
    debug("KwavePlugin::~KwavePlugin(), done.");
}

//***************************************************************************
QStrList *KwavePlugin::setup(QStrList *)
{
    QStrList *result = new QStrList();
    ASSERT(result);
    return result;
}

//***************************************************************************
int KwavePlugin::start(QStrList &)
{
    MutexGuard lock(m_thread_lock);
    return 0;
}

//***************************************************************************
int KwavePlugin::stop()
{
    MutexGuard lock(m_thread_lock);
    if (m_thread) {
	if (m_thread->running()) m_thread->wait(5000);
	if (m_thread->running()) m_thread->stop();
	if (m_thread->running()) m_thread->wait(1000);
	if (m_thread->running()) {
	    // show a message box
	    warning("KwavePlugin::stop(): stale thread !");
	}
	debug("KwavePlugin::stop(): deleting thread");
	delete m_thread;
	debug("KwavePlugin::stop(): thread deleted");
	m_thread = 0;
    }
    return 0;
}

//***************************************************************************
int KwavePlugin::execute(QStrList &params)
{
    MutexGuard lock(m_thread_lock);

    m_thread = new Asynchronous_Object_with_1_arg<KwavePlugin, QStrList>(
	this, &KwavePlugin::run,params);
    ASSERT(m_thread);
    if (!m_thread) return -ENOMEM;

    debug("KwavePlugin::execute(): activating thread");
    m_thread->start();

    // sometimes the signal proxies remain blocked until an initial
    // X11 event occurs and thus might block the thread :-(
    QApplication::syncX();

    return 0;
}

//***************************************************************************
void KwavePlugin::run(QStrList)
{
}

//***************************************************************************
void KwavePlugin::close()
{
    debug("KwavePlugin::close() [slot]");
    stop();
    emit sigClosed(this, true);
}

//***************************************************************************
PluginManager &KwavePlugin::manager()
{
    return m_context.manager;
}

//***************************************************************************
QWidget *KwavePlugin::parentWidget()
{
    return &(m_context.top_widget);
}

//***************************************************************************
const QString &KwavePlugin::signalName()
{
    return (m_context.top_widget.getSignalName());
}

//***************************************************************************
unsigned int KwavePlugin::signalLength()
{
    return manager().signalLength();
}

//***************************************************************************
unsigned int KwavePlugin::signalRate()
{
    return manager().signalRate();
}

//***************************************************************************
const QArray<unsigned int> KwavePlugin::selectedChannels()
{
    return manager().selectedChannels();
}

//***************************************************************************
unsigned int KwavePlugin::selection(unsigned int *left, unsigned int *right)
{
    int l = manager().selectionStart();
    int r = manager().selectionEnd();
    if (left)  *left  = l;
    if (right) *right = r;
    return r-l+1;
}

//***************************************************************************
int KwavePlugin::singleSample(unsigned int channel, unsigned int offset)
{
    return manager().singleSample(channel, offset);
}

//***************************************************************************
int KwavePlugin::averageSample(unsigned int offset,
                               const QArray<unsigned int> *channels)
{
    return manager().averageSample(offset, channels);
}

//***************************************************************************
QBitmap *KwavePlugin::overview(unsigned int width, unsigned int height,
                               unsigned int offset, unsigned int length)
{
    return manager().overview(width, height, offset, length);
}

//***************************************************************************
void KwavePlugin::yield()
{
    pthread_testcancel();
    sched_yield();
}

//***************************************************************************
void *KwavePlugin::handle()
{
    return m_context.handle;
}

//***************************************************************************
void KwavePlugin::zoom2string(char *buf, unsigned int bufsize, double percent)
{
    if (percent < 1.0) {
	char format[128];
	int digits = (int)ceil(1.0 - log10(percent));

	snprintf(format, sizeof(format), "%%0.%df %%%%", digits);
	snprintf(buf, bufsize, format, percent);
    } else if (percent < 10.0) {
	snprintf(buf, bufsize, "%0.1f %%", percent);
    } else if (percent < 1000.0) {
	snprintf(buf, bufsize, "%0.0f %%", percent);
    } else {
	snprintf(buf, bufsize, "x %d", (int)(percent / 100.0));
    }
}

//***************************************************************************
QString KwavePlugin::ms2string(double ms)
{
    char buf[128];
    int bufsize = 128;

    if (ms < 1.0) {
	char format[128];
	// limit to 6 digits, use 0.0 for exact zero
	int digits = (ms != 0.0) ? (int)ceil(1.0 - log10(ms)) : 1;
	if ( (digits < 0) || (digits > 6)) digits = 6;

	snprintf(format, sizeof(format), "%%0.%df ms", digits);
	snprintf(buf, bufsize, format, ms);
    } else if (ms < 1000.0) {
	snprintf(buf, bufsize, "%0.1f ms", ms);
    } else {
	int s = (int)round(ms / 1000.0);
	int m = (int)floor(s / 60.0);
	
	if (m < 1) {
	    char format[128];
	    int digits = (int)ceil(7.0 - log10(ms));
	    snprintf(format, sizeof(format), "%%0.%df s", digits);
	    snprintf(buf, bufsize, format, ms / 1000.0);
	} else {
	    snprintf(buf, bufsize, "%02d:%02d min", m, s % 60);
	}
    }

    QString result(buf);
    return result;
}

//***************************************************************************
//***************************************************************************
/* end of libgui/KwavePlugin.cpp */
