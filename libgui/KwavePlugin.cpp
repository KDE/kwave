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
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/Signal.h"
#include "libkwave/Track.h"
#include "libgui/KwavePlugin.h"
#include "libgui/PluginContext.h"

#include "kwave/PluginManager.h"
#include "kwave/TopWidget.h"
#include "kwave/SignalManager.h"

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
}

//***************************************************************************
const QString &KwavePlugin::name()
{
    return m_context.name;
}

//***************************************************************************
const QString &KwavePlugin::version()
{
    return m_context.version;
}

//***************************************************************************
const QString &KwavePlugin::author()
{
    return m_context.author;
}

//***************************************************************************
void KwavePlugin::load(QStringList &)
{
    debug("KwavePlugin(%s): load()", m_context.name.data());
}

//***************************************************************************
QStringList *KwavePlugin::setup(QStringList &)
{
    QStringList *result = new QStringList();
    ASSERT(result);
    return result;
}

//***************************************************************************
int KwavePlugin::start(QStringList &)
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
	delete m_thread;
	m_thread = 0;
    }
    return 0;
}

//***************************************************************************
int KwavePlugin::execute(QStringList &params)
{
    MutexGuard lock(m_thread_lock);

    m_thread = new Asynchronous_Object_with_1_arg<KwavePlugin, QStringList>(
	this, &KwavePlugin::run,params);
    ASSERT(m_thread);
    if (!m_thread) return -ENOMEM;

    // start the thread, this executes run()
    m_thread->start();

    // sometimes the signal proxies remain blocked until an initial
    // X11 event occurs and thus might block the thread :-(
    QApplication::syncX();

    return 0;
}

//***************************************************************************
void KwavePlugin::run(QStringList)
{
    debug("KwavePlugin::run");
}

//***************************************************************************
void KwavePlugin::close()
{
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
const QArray<unsigned int> KwavePlugin::selectedTracks()
{
    return manager().selectedTracks();
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
void KwavePlugin::openSampleReaderSet(
    QVector<SampleReader> &readers,
    const QArray<unsigned int> &track_list,
    unsigned int first, unsigned int last)
{
    unsigned int count = track_list.count();
    unsigned int track;
    readers.clear();
    readers.resize(count);
    Signal &signal(m_context.top_widget.signalManager().signal());

    for (unsigned int i=0; i < count; i++) {
	track = track_list[i];
	SampleReader *s = signal.openSampleReader(track, first, last);
	ASSERT(s);
	readers.insert(i, s);
    }

    readers.setAutoDelete(true);
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
QString KwavePlugin::zoom2string(double percent)
{
    QString result = "";

    if (percent < 1.0) {
	int digits = (int)ceil(1.0 - log10(percent));
	QString format;
	format = "%0."+format.setNum(digits)+"f %%";
	result = format.sprintf(format, percent);
    } else if (percent < 10.0) {
	result = result.sprintf("%0.1f %%", percent);
    } else if (percent < 1000.0) {
	result = result.sprintf("%0.0f %%", percent);
    } else {
	result = result.sprintf("x %d", (int)rint(percent / 100.0));
    }
    return result;
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
