/*************************************************************************
    KwaveFileDialog.cpp  -  enhanced KFileDialog
                             -------------------
    begin                : Thu May 30 2002
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

#include <qstring.h>
#include <qstringlist.h>

#include <kapplication.h>
#include <kconfig.h>

#include "KwaveFileDialog.h"

//***************************************************************************
KwaveFileDialog::KwaveFileDialog(const QString &startDir,
    const QString &filter, QWidget *parent, const char *name, bool modal)
    :KFileDialog(startDir, filter, parent, name, modal),
     m_config_group(0), m_last_url(0), m_last_ext(0)
{
    if ( (startDir.startsWith(":<") || startDir.startsWith("::<")) &&
	 (startDir.right(1) == ">"))
    {
	// configuration key given -> load initial settings
	QString section = startDir.right(startDir.length() -
	    startDir.find("<") - 1);
	section = "KwaveFileDialog-"+section.left(section.length()-1);
	loadConfig(section);
    }

    // apply the last url if found one
    if (m_last_url.length()) setURL(m_last_url);

    // put the last extension to the top of the list
    // and thus make it selected
    if (m_last_ext.length() && filter.length()) {
	QStringList filter_list = QStringList::split("\n", filter);
	QStringList::Iterator it;
	for (it = filter_list.begin(); it != filter_list.end(); ++it) {
	    QString f = (*it);
	    if (f.contains("|")) f = f.left(f.find("|"));
	    QStringList extensions = QStringList::split(" ", f);
	    if (extensions.contains(m_last_ext)) {
		f = (*it);
		filter_list.remove(it);
		filter_list.prepend(f);
		QString new_filter = filter_list.join("\n");
		setFilter(new_filter);
		break;
	    }
	}
    }

    // save the configuration when the dialog has been accepted
    connect(this, SIGNAL(finished()), this, SLOT(saveConfig()));
}

//***************************************************************************
void KwaveFileDialog::loadConfig(const QString &section)
{
    if (!section.length()) return;
    KConfig *cfg = KApplication::kApplication()->config();
    ASSERT(cfg);
    if (!cfg) return;

    cfg->setGroup(section);
    m_config_group = section;
    m_last_url = cfg->readEntry("last_url", "");
    m_last_ext = cfg->readEntry("last_ext", "");
}

//***************************************************************************
void KwaveFileDialog::saveConfig()
{
    if (!m_config_group.length()) return;

    KConfig *cfg = KApplication::kApplication()->config();
    ASSERT(cfg);
    if (!cfg) return;

    // store the last URL
    m_last_url = baseURL().prettyURL();

    // store the last extension if present
    QFileInfo file(selectedURL().fileName());
    QString extension = file.extension(false);
    if (extension.length()) {
	m_last_ext = "*."+extension;
    }

    cfg->setGroup(m_config_group);
    cfg->writeEntry("last_url", m_last_url);
    cfg->writeEntry("last_ext", m_last_ext);
    cfg->sync();
}

//***************************************************************************
//***************************************************************************
