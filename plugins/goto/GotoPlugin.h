/***************************************************************************
           GotoPlugin.h  -  Plugin for moving the view to a certain position
                             -------------------
    begin                : Sat Dec 06 2008
    copyright            : (C) 2008 by Thomas Eschenbacher
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

#ifndef _GOTO_PLUGIN_H_
#define _GOTO_PLUGIN_H_

#include "config.h"

#include <QtCore/QString>

#include "GotoPluginBase.h"

namespace Kwave
{

    class PluginContext;

    class GotoPlugin: public Kwave::GotoPluginBase
    {
    public:

	/** Constructor */
	explicit GotoPlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~GotoPlugin();

	/** Returns the name of the plugin. */
	virtual QString name() const;

    protected:

	/** Returns the command to be emitted */
	virtual QString command() const;

	/** Returns the title of the dialog */
	virtual QString title() const;

    };
}

#endif /* _GOTO_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
