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

#ifndef ABOUT_PLUGIN_H
#define ABOUT_PLUGIN_H

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
	 * @param parent reference to our plugin manager
	 * @param args argument list [unused]
	 */
 	AboutPlugin(QObject *parent, const QVariantList &args);

	/** Destructor */
	virtual ~AboutPlugin() {}

	/** Returns the name of the plugin. */
	QString name() const Q_DECL_OVERRIDE { return _("about"); }

	/**
	* shows the about dialog,
	* @see Kwave::Plugin::start()
	*/
	int start(QStringList &params) Q_DECL_OVERRIDE;

    };
}

#endif /* ABOUT_PLUGIN_H */

//***************************************************************************
//***************************************************************************
