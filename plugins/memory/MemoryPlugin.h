/***************************************************************************
         MemoryPlugin.h  -  setup of Kwave's memory management
                             -------------------
    begin                : Sun Aug 05 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef _MEMORY_PLUGIN_H_
#define _MEMORY_PLUGIN_H_

#include "config.h"
#include <qobject.h>
#include <qstring.h>

#include "libgui/KwavePlugin.h"

class QStringlist;
class PluginContext;

/**
 * @class MemoryPlugin
 * Setup plugin for Kwave's memory manager.
 */
class MemoryPlugin: public KwavePlugin
{
    Q_OBJECT

public:

    /** Constructor */
    MemoryPlugin(PluginContext &c);

    /** Destructor */
    virtual ~MemoryPlugin();

    /**
     * Gets called when the plugin is first loaded.
     */
    virtual void load(QStringList &params);

    /** @see KwavePlugin::setup() */
    virtual QStringList *setup(QStringList &previous_params);

    /**
     * This plugin needs to be persistent!
     * @see KwavePlugin::isPersistent()
     */
    virtual bool isPersistent() { return true; };

protected:

    /**
     * Interpretes a given parameter list and sets up internal
     * parameters accordingly.
     * @param params reference to a QStringList with parameters
     * @return 0 if ok, or an error code if failed
     */
    int interpreteParameters(QStringList &params);

private:

    /** If true, the physical memory is limited */
    bool m_physical_limited;

    /**
     * Maximum amount of physical memory [MB]. Only of interest if
     * m_physical_limited is set to true, otherwise reflects the
     * user's last setting.
     */
    unsigned int m_physical_limit;

    /** If true, virtual memory is enabled */
    bool m_virtual_enabled;

    /**
     * If true, virtual memory is limited. Only of interest if
     * m_virtual_enabled is set to true, otherwise reflects the
     * user's last setting.
     */
    bool m_virtual_limited;

    /**
     * Maximum amount of virtual memory [MB]. Only of interest if
     * m_virtual_enabled and m_virtual_limited are set to true, otherwise
     * reflects the user's last setting.
     */
    unsigned int m_virtual_limit;

    /** Directory for page files. Default = /tmp */
    QString m_virtual_directory;

};

#endif /* _MEMORY_PLUGIN_H_ */
