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

#ifndef _KWAVE_APPLICATION_CONTEXT_H_
#define _KWAVE_APPLICATION_CONTEXT_H_

#include "config.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>

#include <kdemacros.h>

class QApplication;
class QWidget;

namespace Kwave
{

    class App;
    class MainWidget;
    class PluginManager;
    class SignalManager;
    class TopWidget;

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

	/** returns a reference to the global Kwave application */
	Kwave::App      &application() const;

	/** returns a pointer to the instance's toplevel window */
	Kwave::TopWidget     *topWidget() const;

	/**
	 * returns a pointer to the main widget of this context
	 */
	Kwave::MainWidget    *mainWidget() const;

	/** returns a pointer to the signal manager of this context */
	Kwave::SignalManager *signalManager() const;

	/** returns a pointer to the plugin manager of this context */
	PluginManager *pluginManager() const;

    public slots:

	/**
	 * Execute a Kwave text command
	 * @param command a text command
	 * @return zero if succeeded or negative error code if failed
	 */
	int executeCommand(const QString &command);

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
    };

}

#endif /* _KWAVE_APPLICATION_CONTEXT_H_ */

//***************************************************************************
//***************************************************************************
