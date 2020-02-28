/***************************************************************************
               Plugin.h  -  base class of all Kwave plugins
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

#ifndef PLUGIN_H
#define PLUGIN_H

#include "config.h"

#include <QAtomicInt>
#include <QtGlobal>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QVariantList>
#include <QVector>

#include <KPluginFactory>

#include "libkwave/Runnable.h"
#include "libkwave/Sample.h"
#include "libkwave/String.h"

class QProgressDialog;
class QStringList;
class QVariant;

/* forward declarations, to make clang happy */
Q_EXTERN_C Q_DECL_EXPORT const char *qt_plugin_query_metadata();
Q_EXTERN_C Q_DECL_EXPORT QT_PREPEND_NAMESPACE(QObject) *qt_plugin_instance();

/**
 * @def KWAVE_PLUGIN(name,class)
 * @param name short internal name of the plugin, used for the library
 *             file name, in most cases identical to the directory name
 * @param class name of the C++ class that implements the plugin, without
 *              namespace
 */
#define KWAVE_PLUGIN(name,class)                               \
    K_PLUGIN_FACTORY_WITH_JSON(kwaveplugin_##name##_factory,   \
                               "kwaveplugin_"#name".json",     \
                               registerPlugin<Kwave::class>();)

namespace Kwave
{

    class ConfirmCancelProxy;
    class PluginManager;
    class SampleReader;
    class SignalManager;
    class TopWidget;
    class WorkerThread;

    /**
     * Generic class that should be used for all types of Kwave plugins.
     * This interface is the only one that should be used, it provides
     * all necessary functions to access the functionality of the main
     * Kwave program.
     */
    class Q_DECL_EXPORT Plugin: public QObject, public Kwave::Runnable
    {
	Q_OBJECT

    public:

	/**
	 * Constructor
	 * @param parent pointer to our plugin manager
	 * @param args argument list for initializing the plugin
	 */
	Plugin(QObject *parent, const QVariantList &args);

	/**
	 * Destructor.
	 */
        virtual ~Plugin() Q_DECL_OVERRIDE;

	/** Returns the name of the plugin. */
	virtual QString name() const;

	/** Returns the description of the plugin (already translated). */
	virtual QString description() const;

	/**
	 * Returns a text for the progress dialog if enabled.
	 * (already be localized)
	 */
	virtual QString progressText();

	/**
	 * Returns true if the plugin can be closed.
	 * The default implementation returns "!isRunning()"
	 */
	virtual bool canClose() const;

	/**
	 * Returns true if the plugin has a running thread.
	 */
	bool isRunning() const;

	/**
	 * Returns true if the plugin should stop, e.g. when the
	 * user has pressed "cancel"
	 */
	bool shouldStop() const;

	/**
	 * Called after the plugin has been loaded into memory. This is
	 * useful for plugins that don't use start() and execute(),
	 * maybe for some persistent plugins like playback and record.
	 * The default implementation does nothing.
	 */
	virtual void load(QStringList &params);

	/**
	 * Called before the plugin gets unloaded. Can be used by a plugin
	 * to do some cleanup tasks before it gets unloaded from memory.
	 */
	virtual void unload();

	/**
	 * Sets up all necessary parameters for executing the plugin. Could
	 * be overwritten and show a dialog box. This only will be called if
	 * currently no parameters are given from the function that loads
	 * the plugin (the normal case) but not in the context of replaying
	 * a previously recorded macro.
	 * @param previous_params the parameters of a previous call, could
	 *        be used to initialize the controls of a setup dialog
	 * @return a string list with all parameters, an empty list (if nothing
	 *         has to be set up) or null if the setup (dialog) has been
	 *         aborted and the plugin should not get executed
	 */
	virtual QStringList *setup(QStringList &previous_params);

	/**
	 * Is called from the main program before the run() function and can
	 * be overwritten to show a window or initialize some things before
	 * the run() function gets called.
	 * @param params list of strings with parameters
	 * @return an error code if the execution failed or zero if everything
	 *         was ok.
	 */
	virtual int start(QStringList &params);

	/**
	 * Stops any threads and is called from the close() function and
	 * the plugin's destructor.
	 */
	virtual int stop();

	/**
	 * Gets called from the plugin's execute function and should be
	 * overwritten to perform some action. This function runs in a
	 * separate thread!
	 * @param params list of strings with parameters
	 * @see  sigDone
	 */
	virtual void run(QStringList params);

	/**
	 * Returns a reference to the manager of this plugin.
	 */
	Kwave::PluginManager &manager() const;

	/** Returns a reference to the current signal manager */
	Kwave::SignalManager &signalManager();

	/**
	 * Returns the parent widget of the plugin. This normally should be
	 * a TopWidget of the Kwave main program.
	 */
	QWidget *parentWidget() const;

	/**
	 * Returns the name of the current signal. This can be used to set the
	 * caption of a plugin's main window. If no signal is currently loaded
	 * the returned string is empty.
	 */
	QString signalName();

	/**
	 * Returns the length of the current signal in samples. If no signal is
	 * present the return value will be zero.
	 */
	virtual sample_index_t signalLength();

	/**
	 * Returns the sample rate of the current signal. If no signal is
	 * present the return value will be zero.
	 */
	virtual double signalRate();

	/**
	 * Returns an array of indices of currently selected channels.
	 */
	virtual const QVector<unsigned int> selectedTracks();

	/**
	 * Returns the left and right sample index of the current selection
	 * in samples from 1 to (size-1). The left and right samples
	 * are included in the selection and might be equal. The left
	 * is always less or equal than the right. Note that there is
	 * always at least one sample selected!
	 *
	 * @param tracks received a list of selected tracks
	 *               (optional or null-pointer)
	 * @param left receives the first selected sample
	 *             (optional or null-pointer)
	 * @param right receives the last selected sample
	 *              (optional or null-pointer)
	 * @param expand_if_empty if set to true, the selection will be made
	 *                        equal to the whole signal if left==right
	 * @return the number of selected samples (right-left+1) [1..length]
	 */
	virtual sample_index_t selection(
	    QVector<unsigned int> *tracks = Q_NULLPTR,
            sample_index_t *left = Q_NULLPTR,
            sample_index_t *right = Q_NULLPTR,
	    bool expand_if_empty = false);

	/**
	 * Sets the current start and length of the selection to new values.
	 * @param offset index of the first sample
	 * @param length number of samples
	 */
	virtual void selectRange(sample_index_t offset, sample_index_t length);

	/**
	 * Migrates this plugin instance to the currently active file context.
	 * This might be necessary for plugins that create a new file context
	 * and then expect that all operations are executed in the context of
	 * that new file context. Example: the record plugin creates a new
	 * context and does recording into that new created file context.
	 */
	virtual void migrateToActiveContext();

    protected:

	friend class Kwave::PluginManager;

	/**
	 * Gets called to execute the plugin's run function in a separate
	 * thread.
	 * @param params list of strings with parameters
	 * @bug the return value is never evaluated
	 */
	int execute(QStringList &params);

	/** emits a sigCommand() */
	void emitCommand(const QString &command);

	/** increments the usage counter */
	void use();

	/** assign this plugin to a new plugin manager (when migrating) */
	void setPluginManager(Kwave::PluginManager *new_plugin_manager);

    signals:

	/**
	 * will be emitted when the "run" function starts
	 * @see run
	 */
	void sigRunning(Kwave::Plugin *plugin);

	/**
	 * will be emitted when the "run" function has finished
	 * @see run
	 */
	void sigDone(Kwave::Plugin *plugin);

	/** will be emitted in the plugin's destructor */
	void sigClosed(Kwave::Plugin *p);

	/** can be used by plugins to execute toplevel commands */
	void sigCommand(const QString &command);

	/**
	 * Sets the text of the progress dialog
	 * @param text new progress bar text, already be localized
	 */
	void setProgressText(const QString &text);

    public slots:

	/**
	 * Switches the support for a progress dialog on [default] or off
	 */
	virtual void setProgressDialogEnabled(bool enable);

	/**
	 * update the progress dialog
	 * @param progress the current progress in percent [0...100]
	 */
	virtual void updateProgress(qreal progress);

	/**
	 * called when the user has pressed "Cancel" in the progress
	 * dialog and also has confirmed the cancel confirmation
	 * message box.
	 */
	virtual void cancel();

	/**
	 * Called to close the plugin. This will be called from the plugin
	 * manager and can as well be used from inside the plugin if it
	 * wishes to close itself.
	 */
	virtual void close();

	/** decrements the usage counter */
	void release();

    private slots:

	/** closes the progress dialog and the confirm/cancel proxy */
	void closeProgressDialog(Kwave::Plugin *);

	/** updates the progress bar, triggered by timer */
	void updateProgressTick();

    protected:

	friend class Kwave::WorkerThread;

	/** Wrapper for run() that contains a call to release() */
        virtual void run_wrapper(const QVariant &params) Q_DECL_OVERRIDE;

    private:

	/** reference to the plugin manager */
	Kwave::PluginManager *m_plugin_manager;

	/** name of the plugin, for undo/redo */
	QString m_name;

	/** description of the plugin, for GUI purposes, captions etc */
	QString m_description;

	/**
	 * Thread that executes the run() member function.
	 */
	Kwave::WorkerThread *m_thread;

	/** Mutex for control over the thread */
	QMutex m_thread_lock;

	/** determines whether a progress dialog should be used in run() */
	bool m_progress_enabled;

	/** flag for stopping the process (stop if set to non-zero) */
	QAtomicInt m_stop;

	/** a progress dialog, if the audio processing takes longer... */
	QProgressDialog *m_progress;

	/**
	 * proxy dialog that asks for confirmation if the user
	 * pressed cancel in the progress dialog
	 */
	Kwave::ConfirmCancelProxy *m_confirm_cancel;

	/** Usage counter */
	unsigned int m_usage_count;

	/** Mutex for locking the usage counter */
	QMutex m_usage_lock;

	/** timer for updating the progress dialog */
	QTimer m_progress_timer;

	/** latest progress value [percent] */
	qreal m_current_progress;

	/** Mutex for locking the progress bar */
	QMutex m_progress_lock;

    };

}

#endif /* PLUGIN_H */

//***************************************************************************
//***************************************************************************
