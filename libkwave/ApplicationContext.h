/***************************************************************************
   ApplicationContext.h  -  Context of one Kwave instance
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

#include <QPointer>

#include <kdemacros.h>

class QApplication;
class QWidget;
class KwaveApp;
class TopWidget;

namespace Kwave
{

    class PluginManager;
    class SignalManager;

    class KDE_EXPORT ApplicationContext
    {
    public:
	/**
	 * Constructor. Creates a new toplevel window, signal manager,
	 * plugin manager and so on.
	 * @param app reference to the Kwave application
	 * @note implementation is in kwave/ApplicationContext.cpp
	 */
	ApplicationContext(KwaveApp &app);

	/**
	 * Destructor
	 * @note implementation is in kwave/ApplicationContext.cpp
	 */
	~ApplicationContext();

	/**
	 * initializes the instance
	 * @return true if successful
	 * @note implementation is in kwave/ApplicationContext.cpp
	 */
	bool init();

	/** returns a reference to the global Kwave application */
	KwaveApp      &application();

	/** returns a pointer to the instance's toplevel window */
	TopWidget     *topWidget();

	/** returns a pointer to the instance's signal manager */
	Kwave::SignalManager *signalManager();

	/** returns a pointer to the instance's plugin manager */
	PluginManager *pluginManager();

    private:

	/** reference to the global Kwave application object */
	KwaveApp &m_application;

	/** instance of our toplevel window */
	QPointer<TopWidget> m_top_widget;

	/** instance of our signal manager */
	QPointer<Kwave::SignalManager> m_signal_manager;

	/** instance of our plugin manager */
	QPointer<Kwave::PluginManager> m_plugin_manager;
    };

}

#endif /* _KWAVE_APPLICATION_CONTEXT_H_ */

//***************************************************************************
//***************************************************************************
