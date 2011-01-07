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

#include "config.h"

#include <QFileInfo>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QUrl>

#include <kapplication.h>
#include <kconfig.h>
#include <kfilefiltercombo.h>

#include "KwaveFileDialog.h"

//***************************************************************************
KwaveFileDialog::KwaveFileDialog(const QString &startDir,
    const QString &filter, QWidget *parent, bool modal,
    const QString last_url, const QString last_ext)
    :KFileDialog(startDir, filter, parent),
     m_config_group(), m_last_url(last_url), m_last_ext(last_ext)
{
    setModal(modal);

    if ( (startDir.startsWith(":<") || startDir.startsWith("::<")) &&
	 (startDir.right(1) == ">"))
    {
	// configuration key given -> load initial settings
	QString section = startDir.right(startDir.length() -
	    startDir.indexOf("<") - 1);
	section = "KwaveFileDialog-"+section.left(section.length()-1);
	loadConfig(section);
    }

    // apply the last url if found one
    if (m_last_url.length()) setUrl(KUrl(m_last_url));

    // if a filename was passed, try to re-use it
    if (last_url.length()) {
	QFileInfo fi(last_url);
	setSelection(fi.completeBaseName());
    }

    // put the last extension to the top of the list
    // and thus make it selected
    if (m_last_ext.length() && filter.length()) {
	QStringList filter_list = filter.split("\n");
	QString best = 0;
	foreach (QString f, filter_list) {
	    if (f.contains("|")) f = f.left(f.indexOf("|"));
	    if (!f.length()) continue;
	    QStringList extensions = f.split(" ");
	    if (extensions.contains(m_last_ext)) {
		if (best.isNull() || (f.length() <= best.length()))
		    best = f;
	    }
	}
	if (!best.isNull()) {
	    filter_list.removeAll(best);
	    filter_list.prepend(best);
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
    const KConfigGroup cfg = KGlobal::config()->group(section);
    m_config_group = section;
    m_last_url = cfg.readEntry("last_url", m_last_url);
    m_last_ext = cfg.readEntry("last_ext", m_last_ext);
}

//***************************************************************************
void KwaveFileDialog::saveConfig()
{
    if (!m_config_group.length()) return;
    if (!selectedUrl().fileName().length()) return; // aborted

    // store the last URL
    m_last_url = baseUrl().prettyUrl();

    // store the last extension if present
    QFileInfo file(selectedUrl().fileName());
    QString extension = file.suffix();
    if (extension.length()) {
	// simple case: file extension
	m_last_ext = "*."+extension;
    } else {
	// tricky case: filename mask
	QString filename = selectedUrl().fileName();
	QString filter = filterWidget()->currentFilter();
	m_last_ext = "";
	foreach (QString mask, filter.split(" ")) {
	    QRegExp regex(mask, Qt::CaseSensitive, QRegExp::Wildcard);
	    if (regex.indexIn(filename) >= 0) {
		m_last_ext = mask;
		break;
	    }
	}
// 	if (!m_last_ext.length())
// 	    no extension given -> since 0.7.7 this is allowed
    }

    KConfigGroup cfg = KGlobal::config()->group(m_config_group);
    cfg.writeEntry("last_url", m_last_url);
    cfg.writeEntry("last_ext", m_last_ext);
    cfg.sync();
}

//***************************************************************************
QString KwaveFileDialog::selectedExtension()
{
    QStringList ext_list = currentFilter().split("; ");
    return *(ext_list.begin());
}

//***************************************************************************
#include "KwaveFileDialog.moc"
//***************************************************************************
//***************************************************************************
