/***************************************************************************
          AboutPlugin.h  -  plugin that shows the Kwave's about dialog
                             -------------------
    begin                : Sun Oct 29 2000
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

#ifndef _ABOUT_PLUGIN_H_
#define _ABOUT_PLUGIN_H_

#include "config.h"

#include "libkwave/Plugin.h"

class QStringList;

namespace Kwave
{
    class AboutPlugin: public Kwave::Plugin
    {
	Q_OBJECT

    public:

	/**
	 * Constructor
	 * @param plugin_manager reference to our plugin manager
	 */
	AboutPlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~AboutPlugin() {};

	/** Returns the name of the plugin. */
	virtual QString name() const;

	/**
	* shows the about dialog,
	* @see Kwave::Plugin::start()
	*/
	virtual int start(QStringList &params);

    };
}

#endif /* _ABOUT_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
