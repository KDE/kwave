/*************************************************************************
          DebugPlugin.h  -  various debug aids
                             -------------------
    begin                : Mon Feb 02 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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

#ifndef _DEBUG_PLUGIN_H_
#define _DEBUG_PLUGIN_H_

#include "config.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "libkwave/Plugin.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"

namespace Kwave
{
    /**
     * This plugin is intended to be used internally for debuggin and
     * verification purposes.
     */
    class DebugPlugin: public Kwave::Plugin
    {
	Q_OBJECT

    public:

	/** Constructor */
	DebugPlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~DebugPlugin();

	/** Returns the name of the plugin. */
	virtual QString name() const;

	/** @see Kwave::Plugin::load() */
	virtual void load(QStringList &params);

	/**
	 * performs the special function
	 * @param params list of strings with parameters
	 */
	virtual void run(QStringList);

    private:

	/** use an intermediate buffer for faster filling */
	Kwave::SampleArray m_buffer;
    };
}

#endif /* _DEBUG_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
