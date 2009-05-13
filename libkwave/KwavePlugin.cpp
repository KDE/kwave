/***************************************************************************
        KwavePlugin.cpp  -  base class of all Kwave plugins
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

#include <QProgressDialog>
#include <QTime>
#include <QVector>
#include <QWidget>

#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>

#include "libkwave/ConfirmCancelProxy.h"
#include "libkwave/KwavePlugin.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/Signal.h"
#include "libkwave/Track.h"
#include "libkwave/PluginContext.h"
#include "libkwave/PluginManager.h"
#include "libkwave/PluginWorkerThread.h"
#include "libkwave/SignalManager.h"

#ifdef DEBUG
#include <execinfo.h> // for backtrace()
#endif

/*
   A plugin can be unloaded through two different ways. The possible
   scenarios are:

   1. WITHOUT a thread
      main thread:  new --- release

   2. WITH a plugin thread
      main thread:    new --- use --- start --- release
      plugin thread:                     --- run --- release

      The plugin can be unloaded either in the main thread or in the
      context of the plugin's thread, depending on what occurs first.

 */

//***************************************************************************
Kwave::Plugin::Plugin(const PluginContext &c)
    :m_context(c),
     m_thread(0),
     m_thread_lock(),
     m_progress_enabled(true),
     m_stop(false),
     m_progress(0),
     m_confirm_cancel(0),
     m_usage_count(0),
     m_usage_lock()
{
    use();
}

//***************************************************************************
Kwave::Plugin::~Plugin()
{
    // inform our owner that we close. This allows the plugin to
    // delete itself
    close();

    {
	QMutexLocker lock(&m_thread_lock);
	if (m_thread) {
	    if (m_thread->isRunning()) m_thread->wait(5000);
	    if (m_thread->isRunning()) m_thread->stop();
	    if (m_thread->isRunning()) m_thread->wait(1000);
	    if (m_thread->isRunning()) {
		// unable to stop the thread
		qWarning("Kwave::Plugin::stop(): stale thread !");
	    }
	    delete m_thread;
	    m_thread = 0;
	}
    }

    if (m_confirm_cancel) delete m_confirm_cancel;
    if (m_progress)       delete m_progress;
}

//***************************************************************************
const QString &Kwave::Plugin::name()
{
    return m_context.m_name;
}

//***************************************************************************
const QString &Kwave::Plugin::version()
{
    return m_context.m_version;
}

//***************************************************************************
const QString &Kwave::Plugin::author()
{
    return m_context.m_author;
}

//***************************************************************************
void Kwave::Plugin::load(QStringList &)
{
}

//***************************************************************************
QStringList *Kwave::Plugin::setup(QStringList &)
{
    QStringList *result = new QStringList();
    Q_ASSERT(result);
    return result;
}

//***************************************************************************
int Kwave::Plugin::start(QStringList &)
{
    QMutexLocker lock(&m_thread_lock);
    m_stop = false;

    Q_ASSERT(!m_progress);
    Q_ASSERT(!m_confirm_cancel);

    // check: start() must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    // create a progress dialog for processing mode (not used for pre-listen)
    if (m_progress_enabled) {
	m_progress = new QProgressDialog(parentWidget());
	Q_ASSERT(m_progress);
    }

    // set up the progress dialog when in processing (not pre-listen) mode
    if (m_progress_enabled && m_progress) {
	unsigned int first, last;
	unsigned int tracks = selectedTracks().count();

	selection(&first, &last, true);
	m_progress->setModal(true);
	m_progress->setVisible(false);
	m_progress->setMinimumDuration(2000);
	m_progress->setAutoClose(true);
	m_progress->setMaximum((last - first + 1) * tracks);
	m_progress->setValue(0);
	m_progress->setLabelText(progressText());
	int h = m_progress->sizeHint().height();
	int w = m_progress->sizeHint().height();
	if (w < 4*h) w = 4*h;
	m_progress->setFixedSize(w, h);

	// use a "proxy" that asks for confirmation of cancel
	m_confirm_cancel = new ConfirmCancelProxy(m_progress,
		0, 0, this, SLOT(cancel()));
	Q_ASSERT(m_confirm_cancel);
	connect(m_progress,       SIGNAL(canceled()),
		m_confirm_cancel, SLOT(cancel()));
	connect(this,             SIGNAL(setProgressText(const QString &)),
	        m_progress,       SLOT(setLabelText(const QString &)),
	        Qt::QueuedConnection);
	connect(this, SIGNAL(sigDone(Kwave::Plugin *)),
		this, SLOT(closeProgressDialog(Kwave::Plugin *)),
		Qt::QueuedConnection);
	m_progress->setVisible(true);
    }

    return 0;
}

//***************************************************************************
QString Kwave::Plugin::progressText()
{
    return i18n("running plugin '%1' ...", name());
}

//***************************************************************************
int Kwave::Plugin::stop()
{
    cancel();

    if (m_thread && m_thread->isRunning() &&
	(QThread::currentThread() == m_thread)) {
	qWarning("Kwave::Plugin::stop(): plugin '%s' called stop() from "\
	         "within it's own worker thread (from run() ?). "\
	         "This would produce a deadlock, dear %s, PLEASE FIX THIS !",
	         name().toLocal8Bit().data(),
		 author().toLocal8Bit().data());

#ifdef DEBUG
	qDebug("pthread_self()=%p, tid=%p",
	       reinterpret_cast<void *>(QThread::currentThread()),
	       reinterpret_cast<void *>(m_thread));
	void *buf[256];
	size_t n = backtrace(buf, 256);
	backtrace_symbols_fd(buf, n, 2);
#endif
	return -EBUSY;
    }

    {
	QMutexLocker lock(&m_thread_lock);
	if (m_thread) {
	    if (m_thread->isRunning()) m_thread->wait(5000);
	    if (m_thread->isRunning()) m_thread->stop();
	    if (m_thread->isRunning()) m_thread->wait(1000);
	    if (m_thread->isRunning()) {
		// unable to stop the thread
		qWarning("Kwave::Plugin::stop(): stale thread !");
	    }
	    delete m_thread;
	    m_thread = 0;
	}
    }
    return 0;
}

//***************************************************************************
void Kwave::Plugin::setProgressDialogEnabled(bool enable)
{
    m_progress_enabled = enable;
}

//***************************************************************************
void Kwave::Plugin::updateProgress(unsigned int progress)
{
//     qDebug("Kwave::Plugin::updateProgress(%u)", progress);

    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    if (m_progress) m_progress->setValue(progress);
}

//***************************************************************************
void Kwave::Plugin::closeProgressDialog(Kwave::Plugin *)
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    if (m_confirm_cancel) delete m_confirm_cancel;
    m_confirm_cancel = 0;

    if (m_progress)       delete m_progress;
    m_progress = 0;
}

//***************************************************************************
void Kwave::Plugin::cancel()
{
    m_stop = true;
}

//***************************************************************************
int Kwave::Plugin::execute(QStringList &params)
{
    QMutexLocker lock(&m_thread_lock);
    m_stop = false;

    m_thread = new Kwave::PluginWorkerThread(this, params);
    Q_ASSERT(m_thread);
    if (!m_thread) return -ENOMEM;

    // start the thread, this executes run()
    m_thread->start();

    return 0;
}

//***************************************************************************
bool Kwave::Plugin::isRunning()
{
    return (m_thread) && m_thread->isRunning();
}

//***************************************************************************
void Kwave::Plugin::run(QStringList)
{
}

//***************************************************************************
void Kwave::Plugin::run_wrapper(QStringList params)
{
    // signal that we are running
    emit sigRunning(this);

    // start time measurement
    QTime t;
    t.start();

    // call the plugin's run function in this worker thread context
    run(params);

    // evaluate the elapsed time
    double seconds = static_cast<double>(t.elapsed()) * 1E-3;
    qDebug("plugin %s done, running for %0.03g seconds",
	name().toLocal8Bit().data(), seconds);

    // emit the "done" signal
    emit sigDone(this);

    m_stop = false;
    release();
}

//***************************************************************************
void Kwave::Plugin::close()
{
    // only call stop() if we are NOT in the worker thread / run function !
    if (m_thread && m_thread->isRunning() &&
        (QThread::currentThread() != m_thread) )
    {
	stop();
    }
}

//***************************************************************************
void Kwave::Plugin::use()
{
    QMutexLocker lock(&m_usage_lock);
    m_usage_count++;
}

//***************************************************************************
void Kwave::Plugin::release()
{
    bool finished = false;

    {
	QMutexLocker lock(&m_usage_lock);
	Q_ASSERT(m_usage_count);
	if (m_usage_count) {
	    m_usage_count--;
	    if (!m_usage_count) finished = true;
	}
    }

    if (finished) emit sigClosed(this);
}

//***************************************************************************
PluginManager &Kwave::Plugin::manager()
{
    return m_context.m_plugin_manager;
}

//***************************************************************************
SignalManager &Kwave::Plugin::signalManager()
{
    return manager().signalManager();
}

//***************************************************************************
QWidget *Kwave::Plugin::parentWidget()
{
    return manager().parentWidget();
}

//***************************************************************************
FileInfo &Kwave::Plugin::fileInfo()
{
    return manager().fileInfo();
}

//***************************************************************************
QString Kwave::Plugin::signalName()
{
    return signalManager().signalName();
}

//***************************************************************************
unsigned int Kwave::Plugin::signalLength()
{
    return manager().signalLength();
}

//***************************************************************************
qreal Kwave::Plugin::signalRate()
{
    return manager().signalRate();
}

//***************************************************************************
const QList<unsigned int> Kwave::Plugin::selectedTracks()
{
    return manager().selectedTracks();
}

//***************************************************************************
unsigned int Kwave::Plugin::selection(unsigned int *left, unsigned int *right,
                                    bool expand_if_empty)
{
    int l = manager().selectionStart();
    int r = manager().selectionEnd();

    // expand to the whole signal if left==right and expand_if_empty is set
    if ((l == r) && (expand_if_empty)) {
	l = 0;
	r = manager().signalLength()-1;
    }

    if (left)  *left  = l;
    if (right) *right = r;
    return r-l+1;
}

//***************************************************************************
void Kwave::Plugin::selectRange(unsigned int offset, unsigned int length)
{
    manager().selectRange(offset, length);
}

//***************************************************************************
void Kwave::Plugin::yield()
{
    pthread_testcancel();
    sched_yield();
}

//***************************************************************************
void *Kwave::Plugin::handle()
{
    return m_context.m_handle;
}

//***************************************************************************
QString Kwave::Plugin::zoom2string(qreal percent)
{
    QString result = "";

    if (percent < 1.0) {
	int digits = static_cast<int>(ceil(1.0 - log10(percent)));
	QString format;
	format = "%0."+format.setNum(digits)+"f %%";
	result = format.sprintf(format.toUtf8(), percent);
    } else if (percent < 10.0) {
	result = result.sprintf("%0.1f %%", percent);
    } else if (percent < 1000.0) {
	result = result.sprintf("%0.0f %%", percent);
    } else {
	result = result.sprintf("x %d",
	    static_cast<int>(rint(percent / 100.0)));
    }
    return result;
}

//***************************************************************************
QString Kwave::Plugin::ms2string(qreal ms, int precision)
{
    char buf[128];
    int bufsize = 128;

    if (ms < 1.0) {
	char format[128];
	// limit precision, use 0.0 for exact zero
	int digits = (ms != 0.0) ? static_cast<int>(ceil(1.0 - log10(ms))) : 1;
	if ( (digits < 0) || (digits > precision)) digits = precision;

	snprintf(format, sizeof(format), "%%0.%df ms", digits);
	snprintf(buf, bufsize, format, ms);
    } else if (ms < 1000.0) {
	snprintf(buf, bufsize, "%0.1f ms", ms);
    } else {
	int s = static_cast<int>(round(ms / 1000.0));
	int m = static_cast<int>(floor(s / 60.0));

	if (m < 1) {
	    char format[128];
	    int digits = static_cast<int>(
		ceil(static_cast<qreal>(precision+1) - log10(ms)));
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
QString Kwave::Plugin::dottedNumber(unsigned int number)
{
    const QString num = QString::number(number);
    QString dotted = "";
    const QString dot = KGlobal::locale()->thousandsSeparator();
    const int len = num.length();
    for (int i=len-1; i >= 0; i--) {
	if ((i != len-1) && !((len-i-1) % 3)) dotted = dot + dotted;
	dotted = num.at(i) + dotted;
    }
    return dotted;
}

//***************************************************************************
void Kwave::Plugin::emitCommand(const QString &command)
{
    manager().enqueueCommand(command);
}

//***************************************************************************
#include "KwavePlugin.moc"
//***************************************************************************
//***************************************************************************
