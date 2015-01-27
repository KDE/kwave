/***************************************************************************
       FileInfoPlugin.h  -  plugin for editing file properties
                             -------------------
    begin                : Fri Jul 19 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#ifndef _FILE_INFO_PLUGIN_H_
#define _FILE_INFO_PLUGIN_H_

#include "config.h"
#include <QtCore/QObject>
#include "libkwave/Plugin.h"

namespace Kwave
{
    class FileInfoPlugin: public Kwave::Plugin
    {
	Q_OBJECT
    public:

	/** Constructor */
	explicit FileInfoPlugin(Kwave::PluginManager &plugin_manager);

	/** virtual Destructor */
	virtual ~FileInfoPlugin();

	/** Returns the name of the plugin. */
	virtual QString name() const;

	/**
	* Shows a dialog for editing file properties.
	* @see Kwave::Plugin::setup
	*/
	virtual QStringList *setup(QStringList &);

    protected:

	/** Applies the new settings */
	void apply(Kwave::FileInfo &new_info);

    };
}

#endif /* _FILE_INFO_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
