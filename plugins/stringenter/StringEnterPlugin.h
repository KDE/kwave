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

	/**
	 * Constructor
	 * @param parent reference to our plugin manager
	 * @param args argument list [unused]
	 */
	StringEnterPlugin(QObject *parent, const QVariantList &args);

	/** Destructor */
	virtual ~StringEnterPlugin();

	/** @see Kwave::Plugin::load() */
        virtual void load(QStringList &params) Q_DECL_OVERRIDE;

	/**
	 * Shows a dialog for a command that will be emitted through
	 * sigCommand if OK has been pressed.
	 * @see Kwave::Plugin::setup
	 */
        virtual QStringList *setup(QStringList &previous_params)
            Q_DECL_OVERRIDE;

    };
}

#endif /* STRING_ENTER_PLUGIN_H */

//***************************************************************************
//***************************************************************************
