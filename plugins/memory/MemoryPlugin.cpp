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

#include <qstring.h>
#include <qstringlist.h>

#include "libkwave/KwavePlugin.h"
#include "kwave/MemoryManager.h"
#include "MemoryPlugin.h"
#include "MemoryDialog.h"

KWAVE_PLUGIN(MemoryPlugin,"memory","Thomas Eschenbacher");

//***************************************************************************
MemoryPlugin::MemoryPlugin(const PluginContext &c)
    :KwavePlugin(c), m_physical_limited(true), m_physical_limit(64),
     m_virtual_enabled(true), m_virtual_limited(true), m_virtual_limit(256),
     m_virtual_directory("/tmp")
{
    i18n("memory");
}

//***************************************************************************
MemoryPlugin::~MemoryPlugin()
{
}

//***************************************************************************
int MemoryPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    if (params.count() != 6) return -EINVAL;

    // parameter #0: physical memory is limited ?
    param = params[0];
    m_physical_limited = param.toUInt(&ok) != 0;
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    // parameter #1: limit for physical memory
    param = params[1];
    m_physical_limit = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    // parameter #2: virtual memory is enabled ?
    param = params[2];
    m_virtual_enabled = param.toUInt(&ok) != 0;
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    // parameter #3: virtual memory is limited ?
    param = params[3];
    m_virtual_limited = param.toUInt(&ok) != 0;
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    // parameter #4: limit for virtual memory
    param = params[4];
    m_virtual_limit = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    // parameter #5: directory for virtual memory files
    param = params[5];
    m_virtual_directory = param;

    return 0;
}

//***************************************************************************
void MemoryPlugin::load(QStringList &params)
{
    interpreteParameters(params);
    applySettings();
}

//***************************************************************************
void MemoryPlugin::applySettings()
{
    MemoryManager &mem = MemoryManager::instance();
    mem.setPhysicalLimit(m_physical_limited ? m_physical_limit : 4096);
    mem.setVirtualLimit(m_virtual_enabled ?
                       (m_virtual_limited ? m_virtual_limit : 4096) :
		       0);
    mem.setSwapDirectory(m_virtual_directory);
}

//***************************************************************************
QStringList *MemoryPlugin::setup(QStringList &previous_params)
{
    QStringList *result = 0;

    // try to interprete the list of previous parameters, ignore errors
    if (previous_params.count()) interpreteParameters(previous_params);

    MemoryDialog *dlg = new MemoryDialog(parentWidget(), m_physical_limited,
	m_physical_limit, m_virtual_enabled, m_virtual_limited,
	m_virtual_limit, m_virtual_directory);
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
