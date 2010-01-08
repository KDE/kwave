/***************************************************************************
        PluginContext.h  -  collects all references a plugin needs
                             -------------------
    begin                : Fri Jul 28 2000
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

#ifndef _PLUGIN_CONTEXT_H_
#define _PLUGIN_CONTEXT_H_

#include "config.h"

namespace Kwave { class PluginManager; }

#include <QString>

class PluginContext
{
public:
    /** Constructor */
    PluginContext(
	Kwave::PluginManager &plugin_mgr,
	void *mod_handle,
	const QString &mod_name,
	const QString &mod_version,
	const QString &mod_author
    )
        :m_plugin_manager(plugin_mgr),
         m_handle(mod_handle),
         m_name(mod_name),
         m_version(mod_version),
         m_author(mod_author)
    {}

    Kwave::PluginManager &m_plugin_manager;

    void *m_handle;
    QString m_name;
    QString m_version;
    QString m_author;
};

#endif // _PLUGIN_CONTEXT_H_

/* end of libgui/PluginContext.h */
