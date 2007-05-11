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

#include <qfileinfo.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qurl.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kfilefiltercombo.h>

#include "KwaveFileDialog.h"

//***************************************************************************
KwaveFileDialog::KwaveFileDialog(const QString &startDir,
    const QString &filter, QWidget *parent, const char *name, bool modal,
    const QString last_url, const QString last_ext, QWidget *widget)
    :KFileDialog(startDir, filter, parent, name, modal, widget),
     m_config_group(0), m_last_url(last_url), m_last_ext(last_ext)
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

    // if a filename was passed, try to re-use it
    if (last_url.length()) {
	QFileInfo fi(last_url);
	setSelection(fi.baseName(true));
    }

    // put the last extension to the top of the list
    // and thus make it selected
    if (m_last_ext.length() && filter.length()) {
	QStringList filter_list = QStringList::split("\n", filter);
	QStringList::Iterator it;
	QStringList::Iterator best = filter_list.end();
	for (it = filter_list.begin(); it != filter_list.end(); ++it) {
	    QString f = (*it);
	    if (f.contains("|")) f = f.left(f.find("|"));
	    if (!f.length()) continue;
	    QStringList extensions = QStringList::split(" ", f);
	    if (extensions.contains(m_last_ext)) {
		f = (*it);
		if ((best == filter_list.end()) ||
		    (f.length() <= (*best).length()))
		    best = it;
	    }
	}
	if (best != filter_list.end()) {
	    QString f = (*best);
	    filter_list.remove(best);
	    filter_list.prepend(f);
	    QString new_filter = filter_list.join("\n");
	    setFilter(new_filter);
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
    Q_ASSERT(cfg);
    if (!cfg) return;

    cfg->setGroup(section);
    m_config_group = section;
    m_last_url = cfg->readEntry("last_url", m_last_url);
    m_last_ext = cfg->readEntry("last_ext", m_last_ext);
}

//***************************************************************************
void KwaveFileDialog::saveConfig()
{
    if (!m_config_group.length()) return;
    if (!selectedURL().fileName().length()) return; // aborted

    KConfig *cfg = KApplication::kApplication()->config();
    Q_ASSERT(cfg);
    if (!cfg) return;

    // store the last URL
    m_last_url = baseURL().prettyURL();

    // store the last extension if present
    QFileInfo file(selectedURL().fileName());
    QString extension = file.extension(false);
    if (extension.length()) {
	// simple case: file extension
	m_last_ext = "*."+extension;
    } else {
	// tricky case: filename mask
	QString filename = selectedURL().fileName();
	QString filter = filterWidget->currentFilter();
	QStringList masks = QStringList::split(" ", filter);
	QStringList::Iterator it;
	m_last_ext = "";
	for (it = masks.begin(); it != masks.end(); ++it) {
	    QRegExp mask((*it), true, true);
	    if (mask.search(filename, 0) >= 0) {
		m_last_ext = (*it);
		break;
	    }
	}
// 	if (!m_last_ext.length())
// 	    no extension given -> since 0.7.7 this is allowed
    }

    cfg->setGroup(m_config_group);
    cfg->writeEntry("last_url", m_last_url);
    cfg->writeEntry("last_ext", m_last_ext);
    cfg->sync();
}

//***************************************************************************
QString KwaveFileDialog::selectedExtension()
{
    QStringList ext_list = QStringList::split("; ", currentFilter());
    return *(ext_list.begin());
}

//***************************************************************************
#include "KwaveFileDialog.moc"
//***************************************************************************
//***************************************************************************
