/***************************************************************************
    kwave/FileContext.h  -  Context of a Loaded File
			     -------------------
    begin                : 2009-12-31
    copyright            : (C) 2009 by Thomas.Eschenbacher
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

#ifndef KWAVE_FILE_CONTEXT_H
#define KWAVE_FILE_CONTEXT_H

#include "config.h"

#include <QAtomicInt>
#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QTimer>

#include <TODO:kdemacros.h>
#include <TODO:kurl.h>

#include "libkwave/MetaDataList.h"
#include "libkwave/Sample.h"

class QApplication;
class QMdiSubWindow;
class QSize;
class QTextStream;
class QWidget;

class KUrl;

namespace Kwave
{

    class App;
    class MainWidget;
    class Parser;
    class PluginManager;
    class SignalManager;
    class TopWidget;
    class Zoomable;

    class Q_DECL_EXPORT FileContext: public QObject
    {
	Q_OBJECT
    public:
	/**
	 * Constructor. Creates a new toplevel window, signal manager,
	 * plugin manager and so on.
	 * @param app reference to the Kwave application
	 */
	explicit FileContext(Kwave::App &app);

	/**
	 * Destructor
	 */
	virtual ~FileContext();

	/**
	 * initializes the instance
	 * @param top_widget pointer to the toplevel widget
	 * @return true if successful
	 */
	bool init(Kwave::TopWidget *top_widget);

	/**
	 * create a main widget, within the MDI area
	 * or toplevel widget in case of SDI interface
	 * @param preferred_size preferred size of the main widget
	 * @return true if successful, false if failed
	 */
	bool createMainWidget(const QSize &preferred_size);

	/**
	 * migrate this context to a different toplevel widget
	 * @param top_widget pointer to the new toplevel widget
	 */
	void setParent(Kwave::TopWidget *top_widget);

	/** returns a reference to the application instance */
	Kwave::App           &app() const { return m_application; }

	/**
	 * returns a pointer to the main widget of this context
	 */
	QWidget              *mainWidget() const;

	/** returns a pointer to the signal manager of this context */
	Kwave::SignalManager *signalManager() const;

	/** returns a pointer to the plugin manager of this context */
	Kwave::PluginManager *pluginManager() const;

	/**
	 * Returns a pointer to a GUI element that receives zoom info
	 * (the MainWidget)
	 */
	Kwave::Zoomable *zoomable() const;

	/**
	 * Returns whether this context is empty (has a main widget) or not.
	 * @retval true if the context is empty
	 * @retval false if the context has a main widget
	 */
	inline bool isEmpty() const { return m_main_widget.isNull(); }

	/**
	 * Returns whether this context is active or not.
	 * @retval true if the context is active
	 * @retval false if the context is inactive
	 */
	inline bool isActive() const { return m_active; }

	/** returns the name of the signal */
	QString signalName() const;

	/** returns the instance of the loaded file or -1 */
	inline int instanceNr() const { return m_instance_nr; }

	/**
	 * returns a string suitable as window caption
	 * @param with_modified if true, include the "modified" state
	 */
	QString windowCaption(bool with_modified) const;

	/**
	 * Loads a batch file into memory, parses and executes
	 * all commands in it.
	 * @param url URL of the macro (batch file) to be loaded
	 */
	int loadBatch(const KUrl &url);

	/**
	 * Saves the current file.
	 * @return zero if succeeded, non-zero if failed
	 */
	int saveFile();

	/**
	 * Opens a dialog for saving the current file.
	 * @param filename the name of the new file
	 *                 or empty string to open the File/SaveAs dialog
	 * @param selection if set to true, only the current selection
	 *        will be saved
	 * @return zero if succeeded, non-zero if failed
	 */
	int saveFileAs(const QString &filename, bool selection = false);

	/**
	 * Closes the current file.
	 * If the file has been modified and the user wanted to cancel
	 * the close operation, the file will not get closed and the
	 * function returns with false.
	 * @return true if closing is allowed, false if canceled
	 */
	bool closeFile();

	/**
	 * increments the usage count of this context, prevents it from
	 * being deleted
	 */
	void use();

	/**
	 * decrements the usage count of this context, and if it has reached
	 * zero this instance will be deleted (delayed)
	 */
	void release();

    signals:

	/**
	 * emitted when there is a status bar message to show
	 * @param message the status bar message, already localized
	 * @param ms the time in milliseconds to show the message
	 */
	void sigStatusBarMessage(const QString &message, unsigned int ms);

	/**
	 * emitted when the zoom factor of the corresponding main widget
	 * has changed
	 * @param context contains "this"
	 * @param zoom new zoom factor
	 */
	void sigZoomChanged(Kwave::FileContext *context, double zoom);

	/**
	 * emitted when the meta data of the current signal has changed
	 * @param meta_data the new meta data, after the change
	 */
	void sigMetaDataChanged(Kwave::MetaDataList meta_data);

	/**
	 * emits a change in the selected range.
	 * @param offset index of the first selected items
	 * @param length number of selected items
	 */
	void sigSelectionChanged(sample_index_t offset, sample_index_t length);

	/**
	 * Emitted if the state or description of undo/redo has changed. If
	 * undo or redo is unavailable the description will be zero.
	 */
	void sigUndoRedoInfo(const QString &undo, const QString &redo);

	/** emitted when the visible range has changed */
	void sigVisibleRangeChanged(sample_index_t offset,
	                            sample_index_t visible,
	                            sample_index_t total);

	/**
	 * Emitted if the signal changes from non-modified to modified
	 * state or vice-versa.
	 * @param modified true if now modified, false if no longer
	 */
	void sigModified(bool modified);

	/**
	 * emitted when the context is about to be destroyed
	 * (in the context of it's destructor)
	 */
	void destroyed(Kwave::FileContext *context);

    public slots:

	/**
	 * Execute a Kwave text command
	 * @param command a text command
	 * @return zero if succeeded or negative error code if failed
	 */
	int executeCommand(const QString &command);

    private slots:

	/**
	 * called when the current file context has changed
	 * @param context the new file context (can be "this")
	 */
	void contextSwitched(Kwave::FileContext *context);

	/**
	 * emits a sigZoomChanged(this, zoom) when the zoom has changed
	 * in the m_main_widget
	 */
	void forwardZoomChanged(double zoom);

	/**
	 * Called when the playback position has changed
	 * @param offset the current playback position [samples]
	 */
	void updatePlaybackPos(sample_index_t offset);

	/**
	 * Called when the meta data of the current signal has changed
	 * @param meta_data the new meta data, after the change
	 */
	void metaDataChanged(Kwave::MetaDataList meta_data);

	/**
	 * Called when the number of selected samples has changed.
	 * @param offset index of the first selected sample
	 * @param length number of selected samples
	 */
	void selectionChanged(sample_index_t offset, sample_index_t length);

	/**
	 * Called when the undo or redo action has changed.
	 * @param undo description of the last undo action
	 * @param redo description of the last redo action
	 */
	void setUndoRedoInfo(const QString &undo, const QString &redo);

	/**
	 * Called after changes of the currently visible view range
	 * @param offset index of the first visible sample
	 * @param visible number of visible samples
	 * @param total length of the whole signal
	 */
	void visibleRangeChanged(sample_index_t offset,
	                         sample_index_t visible,
	                         sample_index_t total);

	/**
	 * called if the signal now or no longer is modified
	 * @param modified if true: signal now is "modified", otherwise not
	 */
	void modifiedChanged(bool modified);

	/** process the next delayed command from m_delayed_command_queue */
	void processDelayedCommand();

    private:

	class UsageGuard
	{
	public:
	    /**
	     * constructor, increments use count
	     * @param context the file context to use
	     */
	    explicit UsageGuard(Kwave::FileContext *context)
		:m_context(context)
	    {
		if (m_context) m_context->use();
	    }

	    /** destructor, decrements use count of the context */
	    virtual ~UsageGuard()
	    {
		if (m_context) m_context->release();
		m_context = 0;
	    }

	private:
	    QPointer<Kwave::FileContext> m_context;
	};

    private:

	/**
	 * should be called when this context got active, to update
	 * the status bar, toolbar etc.
	 */
	void activated();

	/**
	 * Show a message in the status bar
	 * @param msg the status bar message, already localized
	 * @param ms the time in milliseconds to show the message
	 */
	void statusBarMessage(const QString &msg, unsigned int ms);

	/**
	 * Parses a text stream line by line and executes each line
	 * as a command until all commands are done or the first one fails.
	 * @param stream a QTextStream to read from
	 * @return zero if successful, non-zero error code if a command failed
	 */
	int parseCommands(QTextStream &stream);

	/**
	 * enqueues a command for later execution
	 * @param delay milliseconds to wait before execution
	 * @param command the command to execute
	 */
	void enqueueCommand(unsigned int delay, const QString &command);

	/**
	 * Discards all changes to the current file and loads
	 * it again.
	 * @return zero if succeeded or error code
	 */
	int revert();

	/**
	 * delegate a command to a plugin
	 * @param plugin name of a plugin to delegate the command to
	 * @param parser the parser with the parts of the command
	 * @param param_count required number of parameters
	 */
	int delegateCommand(const char *plugin,
                            Kwave::Parser &parser,
                            unsigned int param_count);

    private:

	/** usage counter [0...n] */
	QAtomicInt m_use_count;

	/** reference to the global Kwave application object */
	Kwave::App &m_application;

	/** instance of our toplevel window */
	QPointer<Kwave::TopWidget> m_top_widget;

	/** instance of our main widget */
	QPointer<Kwave::MainWidget> m_main_widget;

	/** instance of our signal manager */
	QPointer<Kwave::SignalManager> m_signal_manager;

	/** instance of our plugin manager */
	QPointer<Kwave::PluginManager> m_plugin_manager;

	/** if true, this context is active, otherwise it is inactive */
	bool m_active;

	/** last zoom factor */
	double m_last_zoom;

	/** last playback position, only valid if playback is running */
	sample_index_t m_last_playback_pos;

	/** last status bar message */
	QString m_last_status_message_text;

	/** time when the last status message has been shown */
	QElapsedTimer m_last_status_message_timer;

	/** number of milliseconds the status message should be shown */
	unsigned int m_last_status_message_ms;

	/** name of the last undo action */
	QString m_last_undo;

	/** name of the last redo action */
	QString m_last_redo;

	/** last "modified" state of the signal */
	bool m_last_modified;

	/** instance of the loaded file or -1 */
	int m_instance_nr;

	/** timer for delayed commands */
	QTimer m_delayed_command_timer;

	/** queue for delayed execution of commands */
	QList< QPair<unsigned int, QString> > m_delayed_command_queue;

    };

}

#endif /* KWAVE_FILE_CONTEXT_H */

//***************************************************************************
//***************************************************************************
