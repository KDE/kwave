/***************************************************************************
    libkwave/FileContext.h  -  Context of a Loaded File
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

#ifndef _KWAVE_FILE_CONTEXT_H_
#define _KWAVE_FILE_CONTEXT_H_

#include "config.h"

#include <QtCore/QElapsedTimer>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QString>

#include <kdemacros.h>

#include "libkwave/Sample.h"

class QApplication;
class QWidget;

namespace Kwave
{

    class App;
    class MainWidget;
    class PluginManager;
    class SignalManager;
    class TopWidget;
    class Zoomable;

    class KDE_EXPORT FileContext: public QObject
    {
	Q_OBJECT
    public:
	/**
	 * Constructor. Creates a new toplevel window, signal manager,
	 * plugin manager and so on.
	 * @param app reference to the Kwave application
	 * @note implementation is in kwave/FileContext.cpp
	 */
	FileContext(Kwave::App &app);

	/**
	 * Destructor
	 * @note implementation is in kwave/FileContext.cpp
	 */
	virtual ~FileContext();

	/**
	 * initializes the instance
	 * @param top_widget pointer to the toplevel widget
	 * @return true if successful
	 * @note implementation is in kwave/FileContext.cpp
	 */
	bool init(Kwave::TopWidget *top_widget);

	/**
	 * shuts down the instance
	 */
	void close();

	/** returns a pointer to the instance's toplevel window */
	Kwave::TopWidget     *topWidget() const;

	/**
	 * returns a pointer to the main widget of this context
	 */
	Kwave::MainWidget    *mainWidget() const;

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
	 * Returns whether this context is active or not.
	 * @retval true if the context is active
	 * @retval false if the context is inactive
	 */
	bool isActive() const { return m_active; }

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
	 * updates the playback position in the status bar and scrolls the
	 * current view to show the current playback region
	 * @param offset the current playback position [samples]
	 */
	void updatePlaybackPos(sample_index_t offset);

    private:

	/**
	 * should be called when this context got active, to update
	 * the status bar, toolbar etc.
	 */
	void activated();

	/**
	 * should be called when this context got inactive
	 */
	void deactivated();

	/**
	 * Show a message in the status bar
	 * @param msg the status bar message, already localized
	 * @param ms the time in milliseconds to show the message
	 */
	void statusBarMessage(const QString &msg, unsigned int ms);

    private:

	/** reference to the global Kwave application object */
	Kwave::App &m_application;

	/** instance of our toplevel window */
	QPointer<Kwave::TopWidget> m_top_widget;

	/** instance of our main widget */
	Kwave::MainWidget *m_main_widget;

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
    };

}

#endif /* _KWAVE_FILE_CONTEXT_H_ */

//***************************************************************************
//***************************************************************************
