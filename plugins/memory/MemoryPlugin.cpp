/***************************************************************************
       MemoryPlugin.cpp  -  setup of Kwave's memory management
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

#include "config.h"
#include <errno.h>
#include <limits.h>

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "libkwave/MemoryManager.h"
#include "libkwave/Plugin.h"
#include "libkwave/String.h"

#include "MemoryPlugin.h"
#include "MemoryDialog.h"

KWAVE_PLUGIN(Kwave::MemoryPlugin, "memory", "2.4",
             I18N_NOOP("Memory Settings"), "Thomas Eschenbacher");

/** default memory limit for physical memory [MB] */
#define DEFAULT_PHYSICAL_LIMIT 2048

/** default memory limit for swap space [MB] */
#define DEFAULT_VIRTUAL_LIMIT 2048

/** default memory limit for undo memory [MB] */
#define DEFAULT_UNDO_LIMIT 1024

//***************************************************************************
Kwave::MemoryPlugin::MemoryPlugin(Kwave::PluginManager &plugin_manager)
    :Kwave::Plugin(plugin_manager),
     m_physical_limited(true),
     m_physical_limit(DEFAULT_PHYSICAL_LIMIT),
     m_virtual_enabled(true),
     m_virtual_limited(false),
     m_virtual_limit(DEFAULT_VIRTUAL_LIMIT),
     m_virtual_directory(_("/var/tmp")),
     m_undo_limit(DEFAULT_UNDO_LIMIT)
{
}

//***************************************************************************
Kwave::MemoryPlugin::~MemoryPlugin()
{
}

//***************************************************************************
int Kwave::MemoryPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    if (params.count() < 6) return -EINVAL;

    // parameter #0: physical memory is limited ?
    param = params[0];
    m_physical_limited = param.toUInt(&ok) != 0;
    if (!ok) return -EINVAL;

    // parameter #1: limit for physical memory
    param = params[1];
    m_physical_limit = param.toUInt(&ok);
    if (!ok) return -EINVAL;

    // parameter #2: virtual memory is enabled ?
    param = params[2];
    m_virtual_enabled = param.toUInt(&ok) != 0;
    if (!ok) return -EINVAL;

    // parameter #3: virtual memory is limited ?
    param = params[3];
    m_virtual_limited = param.toUInt(&ok) != 0;
    if (!ok) return -EINVAL;

    // parameter #4: limit for virtual memory
    if (m_virtual_limited) {
	param = params[4];
	m_virtual_limit = param.toUInt(&ok);
	if (!ok) return -EINVAL;
    } else {
	m_virtual_limit = INT_MAX;
    }

    // parameter #5: directory for virtual memory files
    param = params[5];
    m_virtual_directory = param;

    // parameter #6: limit for undo/redo
    if (params.count() >= 7)
    {
	param = params[6];
	m_undo_limit = param.toUInt(&ok);
	if (!ok) return -EINVAL;
    }

    return 0;
}

//***************************************************************************
void Kwave::MemoryPlugin::load(QStringList &params)
{
    interpreteParameters(params);
    applySettings();
}

//***************************************************************************
void Kwave::MemoryPlugin::applySettings()
{
    Kwave::MemoryManager &mem = Kwave::MemoryManager::instance();
    mem.setPhysicalLimit(m_physical_limited ? m_physical_limit : 4096);
    mem.setVirtualLimit(m_virtual_enabled ?
                       (m_virtual_limited ? m_virtual_limit : INT_MAX) :
		       0);
    mem.setSwapDirectory(m_virtual_directory);
    mem.setUndoLimit(m_undo_limit);
}

//***************************************************************************
QStringList *Kwave::MemoryPlugin::setup(QStringList &previous_params)
{
    QStringList *result = 0;

    // try to interprete the list of previous parameters, ignore errors
    if (previous_params.count()) interpreteParameters(previous_params);

    Kwave::MemoryDialog *dlg = new Kwave::MemoryDialog(
	parentWidget(), m_physical_limited,
	m_physical_limit, m_virtual_enabled, m_virtual_limited,
	m_virtual_limit, m_virtual_directory, m_undo_limit);
    Q_ASSERT(dlg);
    if (!dlg) return 0;

    if (dlg->exec() == QDialog::Accepted) {
	// get the new parameters and let them take effect
	result = new QStringList();
	Q_ASSERT(result);
	if (result) {
	    dlg->params(*result);
	    interpreteParameters(*result);
	    applySettings();
	}
    };

    delete dlg;
    return result;
}

//***************************************************************************
#include "MemoryPlugin.moc"
//***************************************************************************
//***************************************************************************
