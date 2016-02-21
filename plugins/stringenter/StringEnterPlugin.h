/***************************************************************************
    StringEnterPlugin.h  -  plugin for entering a text command
                             -------------------
    begin                : Sat Mar 14 2015
    copyright            : (C) 2015 by Thomas Eschenbacher
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

#ifndef STRING_ENTER_PLUGIN_H
#define STRING_ENTER_PLUGIN_H

#include "config.h"

#include <QString>
#include <QVector>

#include "libkwave/Plugin.h"

class QStringList;

namespace Kwave
{

    class StringEnterPlugin: public Kwave::Plugin
    {
	Q_OBJECT

    public:

	/** Constructor */
	explicit StringEnterPlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~StringEnterPlugin();

	/** Returns the name of the plugin. */
	virtual QString name() const;

	/** @see Kwave::Plugin::load() */
	virtual void load(QStringList &params);

	/**
	 * Shows a dialog for a command that will be emitted through
	 * sigCommand if OK has been pressed.
	 * @see Kwave::Plugin::setup
	 */
	virtual QStringList *setup(QStringList &previous_params);

    };
}

#endif /* STRING_ENTER_PLUGIN_H */

//***************************************************************************
//***************************************************************************
