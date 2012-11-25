/***************************************************************************
       InsertAtPlugin.h  -  plugin for insertin the clipboard at a position
                             -------------------
    begin                : Thu May 12 2011
    copyright            : (C) 2011 by Thomas Eschenbacher
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

#ifndef _INSERT_AT_PLUGIN_H_
#define _INSERT_AT_PLUGIN_H_

#include "config.h"

#include <QString>

#include "GotoPluginBase.h"

namespace Kwave
{

    class PluginContext;

    class InsertAtPlugin: public Kwave::GotoPluginBase
    {
    public:

	/** Constructor */
	InsertAtPlugin(const Kwave::PluginContext &context);

	/** Destructor */
	virtual ~InsertAtPlugin();

    protected:

	/** Returns the command to be emitted */
	virtual QString command() const;

	/** Returns the title of the dialog */
	virtual QString title() const;

    };
}

#endif /* _INSERT_AT_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
