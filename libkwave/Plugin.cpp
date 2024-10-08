/***************************************************************************
             Plugin.cpp  -  base class of all Kwave plugins
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

#include <new>

#include <QApplication>
#include <QElapsedTimer>
#include <QProgressDialog>
#include <QThread>
#include <QVariantList>
#include <QWidget>

#include <KLocalizedString>

#include "libkwave/ConfirmCancelProxy.h"
#include "libkwave/Plugin.h"
#include "libkwave/PluginManager.h"
#include "libkwave/Sample.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/WorkerThread.h"

#ifdef DEBUG
#include <execinfo.h> // for backtrace()
#endif

/** number of updates of the progress bat per second */
#define PROGRESS_UPDATES_PER_SECOND 4

/** maximum value of the progress bar */
#define PROGRESS_MAXIMUM 1000000

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
Kwave::Plugin::Plugin(QObject *parent, const QVariantList &args)
    :QObject(nullptr),
     Kwave::Runnable(),
     m_plugin_manager(qobject_cast<Kwave::PluginManager *>(parent)),
     m_name(args[0].toString()),
     m_description(args[1].toString()),
     m_thread(nullptr),
     m_thread_lock(),
     m_progress_enabled(true),
     m_stop(0),
     m_progress(nullptr),
     m_confirm_cancel(nullptr),
     m_usage_count(1),
     m_usage_lock(),
     m_progress_timer(),
     m_current_progress(-1),
     m_progress_lock()
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    Q_ASSERT(m_plugin_manager);
    connect(&m_progress_timer, SIGNAL(timeout()),
            this, SLOT(updateProgressTick()),
            Qt::DirectConnection);
}

//***************************************************************************
Kwave::Plugin::~Plugin()
{
    // inform our owner that we close. This allows the plugin to
    // delete itself
    close();

    // lock usage
    QMutexLocker lock_usage(&m_usage_lock);
    {
        QMutexLocker lock_thread(&m_thread_lock);
        if (m_thread) {
            if (m_thread->isRunning()) m_thread->wait(5000);
            if (m_thread->isRunning()) m_thread->stop();
            if (m_thread->isRunning()) m_thread->wait(1000);
            if (m_thread->isRunning()) {
                // unable to stop the thread
                qWarning("Kwave::Plugin::stop(): stale thread !");
            }
            delete m_thread;
            m_thread = nullptr;
        }
    }

    // finally get rid of the confirm/cancel proxy and the progress dialog
    closeProgressDialog(this);
}

//***************************************************************************
void Kwave::Plugin::load(QStringList &)
{
}

//***************************************************************************
void Kwave::Plugin::unload()
{
}

//***************************************************************************
QStringList *Kwave::Plugin::setup(QStringList &)
{
    QStringList *result = new(std::nothrow) QStringList();
    Q_ASSERT(result);
    return result;
}

//***************************************************************************
int Kwave::Plugin::start(QStringList &)
{
    QMutexLocker lock(&m_thread_lock);
    m_stop = false;

    // check: start() must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    // create a progress dialog for processing mode (not used for pre-listen)
    if (m_progress_enabled && !m_progress) {
        m_progress = new(std::nothrow) QProgressDialog(parentWidget());
        Q_ASSERT(m_progress);
    }

    // set up the progress dialog when in processing (not pre-listen) mode
    if (m_progress_enabled && m_progress) {
        sample_index_t first, last;
        QVector<unsigned int> tracks;

        selection(&tracks, &first, &last, true);
        m_progress->setModal(true);
        m_progress->setVisible(false);
        m_progress->setMinimumDuration(2000);
        m_progress->setAutoClose(false);
        m_progress->setMaximum(PROGRESS_MAXIMUM);
        m_progress->setValue(0);
        m_progress->setLabelText(progressText());
        int h = m_progress->sizeHint().height();
        int w = m_progress->sizeHint().height();
        if (w < 4 * h) w = 4 * h;
        m_progress->setFixedSize(w, h);

        // use a "proxy" that asks for confirmation of cancel
        if (!m_confirm_cancel) {
            m_confirm_cancel = new(std::nothrow)
                Kwave::ConfirmCancelProxy(m_progress,
                nullptr, nullptr, this, SLOT(cancel()));
            Q_ASSERT(m_confirm_cancel);
        }
        connect(m_progress,       SIGNAL(canceled()),
                m_confirm_cancel, SLOT(cancel()));
        connect(this,             SIGNAL(setProgressText(QString)),
                m_progress,       SLOT(setLabelText(QString)),
                Qt::QueuedConnection);
        connect(this, SIGNAL(sigDone(Kwave::Plugin*)),
                this, SLOT(closeProgressDialog(Kwave::Plugin*)),
                Qt::QueuedConnection);
        m_progress->setVisible(true);
    }

    return 0;
}

//***************************************************************************
QString Kwave::Plugin::name() const
{
    return m_name;
}

//***************************************************************************
QString Kwave::Plugin::description() const
{
    return m_description;
}

//***************************************************************************
QString Kwave::Plugin::progressText()
{
    return i18n("Running plugin '%1'...", name());
}

//***************************************************************************
int Kwave::Plugin::stop()
{
    cancel();

    if (m_thread && m_thread->isRunning() &&
        (QThread::currentThread() == m_thread)) {
        qWarning("Kwave::Plugin::stop(): plugin '%s' called stop() from "
                 "within it's own worker thread (from run() ?). "
                 "This would produce a deadlock, PLEASE FIX THIS !",
                 DBG(name()));

#ifdef DEBUG
        qDebug("pthread_self()=%p, tid=%p",
               reinterpret_cast<void *>(QThread::currentThread()),
               reinterpret_cast<void *>(m_thread));
        void *buf[256];
        int n = backtrace(buf, 256);
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
            m_thread = nullptr;
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
void Kwave::Plugin::updateProgress(qreal progress)
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

//     qDebug("Kwave::Plugin::updateProgress(%0.1f)", progress);
    QMutexLocker lock(&m_progress_lock);

    // take over the current progress
    // note: no lock needed, this is called in the GUI thread context!
    m_current_progress = qreal(PROGRESS_MAXIMUM / 100.0) * progress;

    // start the timer for updating the progress bar if it is not active
    if (!m_progress_timer.isActive() && m_progress) {
        m_progress_timer.setSingleShot(true);
        m_progress_timer.start(1000 / PROGRESS_UPDATES_PER_SECOND);
    }
}

//***************************************************************************
void Kwave::Plugin::updateProgressTick()
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    QMutexLocker lock(&m_progress_lock);
    if (m_progress) m_progress->setValue(
        Kwave::toInt(qBound<double>(0.0, m_current_progress,
                                    PROGRESS_MAXIMUM)));
}

//***************************************************************************
void Kwave::Plugin::closeProgressDialog(Kwave::Plugin *)
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    QMutexLocker lock(&m_progress_lock);

    // stop the progress timer
    m_progress_timer.stop();
    m_progress_timer.disconnect();

    if (m_confirm_cancel) m_confirm_cancel->disconnect();
    if (m_progress)       m_progress->disconnect();

    // NOTE: as the progress dialog is *modal*, it is higly probable
    //       that this function is called from the context of the event
    //       loop that is provided by the progress dialog
    //       => deleting this object should be done somewhere later...
    if (m_confirm_cancel) {
        m_confirm_cancel->deleteLater();
        m_confirm_cancel = nullptr;
    }

    if (m_progress) {
        m_progress->done(0);
        m_progress->deleteLater();
        m_progress = nullptr;
    }
}

//***************************************************************************
void Kwave::Plugin::cancel()
{
    if (!m_stop)
    {
        QMutexLocker lock(&m_thread_lock);
        m_stop = true;
        if (m_thread) m_thread->cancel();
    }
    emit sigCancel();
}

//***************************************************************************
int Kwave::Plugin::execute(QStringList &params)
{
    QMutexLocker lock(&m_thread_lock);
    m_stop = false;

    m_thread = new(std::nothrow) Kwave::WorkerThread(this, QVariant(params));
    Q_ASSERT(m_thread);
    if (!m_thread) return -ENOMEM;
    connect(this, SIGNAL(sigCancel()), m_thread, SLOT(cancel()),
            Qt::QueuedConnection);

    // increment the use count, it is decremented at the end of
    // the run_wrapper when the thread is finished
    use();

    // start the thread, this executes run()
    m_thread->start();

    return 0;
}

//***************************************************************************
bool Kwave::Plugin::canClose() const
{
    return !isRunning();
}

//***************************************************************************
bool Kwave::Plugin::shouldStop() const
{
    return (m_stop || (m_thread && m_thread->isInterruptionRequested()));
}

//***************************************************************************
bool Kwave::Plugin::isRunning() const
{
    return (m_thread) && m_thread->isRunning();
}

//***************************************************************************
void Kwave::Plugin::run(QStringList params)
{
    Q_UNUSED(params)
}

//***************************************************************************
void Kwave::Plugin::run_wrapper(const QVariant &params)
{
    // signal that we are running
    emit sigRunning(this);

    // start time measurement
    QElapsedTimer t;
    t.start();

    // call the plugin's run function in this worker thread context
    run(params.toStringList());

    // evaluate the elapsed time
    double seconds = static_cast<double>(t.elapsed()) * 1E-3;
    qDebug("plugin %s done, running for %0.03g seconds",
           DBG(name()), seconds);

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
        // check: this must be called from the GUI thread only!
        Q_ASSERT(this->thread() == QThread::currentThread());
        Q_ASSERT(this->thread() == qApp->thread());

        closeProgressDialog(this);
        stop();
    } else if ((QThread::currentThread() == m_thread)) {
        qWarning("Kwave::Plugin::close -> called from worker thread?");
    }
}

//***************************************************************************
void Kwave::Plugin::use()
{
    QMutexLocker lock(&m_usage_lock);
    Q_ASSERT(m_usage_count); // should be at least 1 (from constructor)
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
Kwave::PluginManager &Kwave::Plugin::manager() const
{
    Q_ASSERT(m_plugin_manager);
    return *m_plugin_manager;
}

//***************************************************************************
Kwave::SignalManager &Kwave::Plugin::signalManager()
{
    return manager().signalManager();
}

//***************************************************************************
QWidget *Kwave::Plugin::parentWidget() const
{
    return manager().parentWidget();
}

//***************************************************************************
QString Kwave::Plugin::signalName()
{
    return signalManager().signalName();
}

//***************************************************************************
sample_index_t Kwave::Plugin::signalLength()
{
    return manager().signalLength();
}

//***************************************************************************
double Kwave::Plugin::signalRate()
{
    return manager().signalRate();
}

//***************************************************************************
const QVector<unsigned int> Kwave::Plugin::selectedTracks()
{
    return signalManager().selectedTracks();
}

//***************************************************************************
sample_index_t Kwave::Plugin::selection(QVector<unsigned int> *tracks,
                                        sample_index_t *left,
                                        sample_index_t *right,
                                        bool expand_if_empty)
{
    sample_index_t l = manager().selectionStart();
    sample_index_t r = manager().selectionEnd();

    // expand to the whole signal if left==right and expand_if_empty is set
    if ((l == r) && (expand_if_empty)) {
        l = 0;
        r = manager().signalLength();
        if (r) r--;
    }

    // get the list of selected tracks
    if (tracks) *tracks = signalManager().selectedTracks();

    if (left)  *left  = l;
    if (right) *right = r;
    return (r != l) ? (r - l + 1) : 0;
}

//***************************************************************************
void Kwave::Plugin::selectRange(sample_index_t offset, sample_index_t length)
{
    manager().selectRange(offset, length);
}

//***************************************************************************
void Kwave::Plugin::emitCommand(const QString &command)
{
    manager().enqueueCommand(command);
}

//***************************************************************************
void Kwave::Plugin::migrateToActiveContext()
{
    manager().migratePluginToActiveContext(this);
}

//***************************************************************************
void Kwave::Plugin::setPluginManager(Kwave::PluginManager *new_plugin_manager)
{
    Q_ASSERT(new_plugin_manager);
    if (!new_plugin_manager) return;
    m_plugin_manager = new_plugin_manager;

    if (m_progress)
        m_progress->setParent(m_plugin_manager->parentWidget());
    if (m_confirm_cancel)
        m_confirm_cancel->setParent(m_plugin_manager->parentWidget());
}

//***************************************************************************
//***************************************************************************

#include "moc_Plugin.cpp"
