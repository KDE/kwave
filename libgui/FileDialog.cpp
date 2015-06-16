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

#include <KConfig>
#include <KConfigGroup>
#include <KFileFilterCombo>
#include <KSharedConfig>

#include "libkwave/String.h"

#include "libgui/FileDialog.h"

//***************************************************************************
Kwave::FileDialog::FileDialog(
    const QString &startDir,
    OperationMode mode,
    const QString &filter, QWidget *parent, bool modal,
    const QString last_url, const QString last_ext
)
    :QFileDialog(parent, QString(), startDir, filter),
     m_config_group(), m_last_url(last_url), m_last_ext(last_ext)
{
    setModal(modal);

    if (mode == Saving) {
	setFileMode(QFileDialog::AnyFile);
    } else {
	setFileMode(QFileDialog::ExistingFile);
    }

    QString special_prefix = _("kfiledialog:///");
    if (startDir.startsWith(special_prefix))
    {
	// configuration key given -> load initial settings
	QString section = startDir;
	section = section.remove(0, special_prefix.length());
	if (section.contains(_("/")))
	    section = section.left(section.indexOf(_("/")));
	section.prepend(_("KwaveFileDialog-"));
	loadConfig(section);
    }

    // if a filename was passed, try to re-use it
    if (m_last_url.length() && QUrl(m_last_url).isLocalFile()) {
	QFileInfo file(m_last_url);
	if (QFileInfo(file.path()).exists() || (mode == Saving))
	    setDirectory(QUrl(m_last_url).path());
	if (!file.isDir() && (file.exists() || (mode == Saving)))
	    selectFile(QUrl(m_last_url).fileName());
    }

    // put the last extension to the top of the list
    // and thus make it selected
    if (m_last_ext.length() && filter.length()) {
	QStringList filter_list = filter.split(_("\n"));
	QString best;
	foreach (const QString &filter, filter_list) {
	    QString f(filter);
	    if (f.contains(_("|")))
		f = f.left(f.indexOf(_("|")));
	    if (!f.length()) continue;
	    QStringList extensions = f.split(_(" "));
	    if (extensions.contains(m_last_ext)) {
		if (best.isNull() || (f.length() <= best.length()))
		    best = f;
	    }
	}
	if (best.length()) {
	    filter_list.removeAll(best);
	    filter_list.prepend(best);
	    setNameFilters(filter_list);
	}
    }

    // save the configuration when the dialog has been accepted
    connect(this, SIGNAL(finished()), this, SLOT(saveConfig()));
}

//***************************************************************************
void Kwave::FileDialog::loadConfig(const QString &section)
{
    if (!section.length()) return;
    const KConfigGroup cfg = KSharedConfig::openConfig()->group(section);
    m_config_group = section;
    if (m_last_url.length()) {
	QString last_path = cfg.readEntry("last_url", m_last_url);
	if (last_path.length()) {
	    // take last path, but user defined file name
	    QUrl    url  = QUrl(last_path);
	    QFile f(m_last_url);
	    QString file = f.fileName();
	    url = url.adjusted(QUrl::RemoveFilename);
	    url.setPath(url.path() + file);
	    m_last_url = url.toDisplayString();
	}
    } else {
	m_last_url = cfg.readEntry("last_url", m_last_url);
    }
    if (!m_last_ext.length())
	m_last_ext = cfg.readEntry("last_ext", m_last_ext);

    QByteArray last_state = cfg.readEntry("last_state", QByteArray());
    if (!last_state.isEmpty()) restoreState(last_state);
}

//***************************************************************************
void Kwave::FileDialog::saveConfig()
{
    if (!m_config_group.length()) return;
    if (selectedUrls().isEmpty()) return;

    QString file_name = selectedUrls().first().fileName();
    if (!file_name.length()) return; // aborted

    // store the last URL
    m_last_url = directoryUrl().toDisplayString();

    // store the last extension if present
    QFileInfo file(file_name);
    QString extension = file.suffix();
    if (extension.length()) {
	// simple case: file extension
	m_last_ext = _("*.") + extension;
    } else {
	// tricky case: filename mask
	QString filter = selectedNameFilter();
	m_last_ext = _("");
	foreach (const QString &mask, filter.split(_(" "))) {
	    QRegExp regex(mask, Qt::CaseSensitive, QRegExp::Wildcard);
	    if (regex.indexIn(file_name) >= 0) {
		m_last_ext = mask;
		break;
	    }
	}
    }

    KConfigGroup cfg = KSharedConfig::openConfig()->group(m_config_group);
    cfg.writeEntry("last_url",   m_last_url);
    cfg.writeEntry("last_ext",   m_last_ext);
    cfg.writeEntry("last_state", saveState());
    cfg.sync();
}

//***************************************************************************
QString Kwave::FileDialog::selectedExtension()
{
    QStringList ext_list = selectedNameFilter().split(_("; "));
    return *(ext_list.begin());
}

//***************************************************************************
QUrl Kwave::FileDialog::selectedUrl() const
{
    QList<QUrl> urls = selectedUrls();
    if (urls.isEmpty())
	return QUrl();
    return urls.first();
}

//***************************************************************************
//***************************************************************************
