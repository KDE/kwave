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
    const QUrl last_url, const QString last_ext
)
    :QFileDialog(parent, QString(), QString(), filter),
     m_config_group(), m_last_url(last_url), m_last_ext(last_ext)
{
    const bool saving = (mode == Saving);

    setModal(modal);

    if (saving) {
	setAcceptMode(QFileDialog::AcceptSave);
	setFileMode(QFileDialog::AnyFile);
    } else {
	setAcceptMode(QFileDialog::AcceptOpen);
	setFileMode(QFileDialog::ExistingFile);
    }

    if (m_last_ext.length())
	setDefaultSuffix(m_last_ext);

    QString special_prefix = _("kfiledialog:///");
    if (startDir.startsWith(special_prefix)) {
	// configuration key given -> load initial settings
	QString section = startDir;
	section = section.remove(0, special_prefix.length());
	if (section.contains(_("/")))
	    section = section.left(section.indexOf(_("/")));
	section.prepend(_("KwaveFileDialog-"));
	loadConfig(section);
    }

    // if a filename or directory URL was passed, try to re-use it
    if (!m_last_url.isEmpty() && (m_last_url.isLocalFile() || saving)) {
	QFileInfo file(m_last_url.toLocalFile());
	if (QFileInfo(file.path()).exists() || saving)
	    setDirectoryUrl(m_last_url.adjusted(QUrl::RemoveFilename));
	if (!file.isDir() && (file.exists() || saving))
	    selectFile(m_last_url.fileName());
    }

    // convert the list of file filters into the new Qt-5 style format
    if (filter.length()) {
	QStringList filter_list = filter.split(_("\n"));
	QStringList name_filters;
	QString best_filter;
	QString best_pattern;
	foreach (const QString &filter_item, filter_list) {
	    QString f(filter_item);
	    QString p(_("*"));
	    if (f.contains(_("|"))) {
		int i = f.indexOf(_("|"));
		p = f.left(i);
		f = f.mid(i + 1);
	    }
	    if (!f.length()) continue;
	    f = f + _(" (") + p  + _(")");

	    // put the last extension to the top of the list
	    // and thus make it selected
	    if (m_last_ext.length()) {
		QStringList extensions = p.split(_(" "));
		if (extensions.contains(m_last_ext)) {
		    if (!best_filter.length() ||
			(p.length() <= best_pattern.length())) {
			best_filter  = f;
			best_pattern = p;
		    }
		}
	    }

	    name_filters.append(f);
	}
	if (best_filter.length()) {
	    name_filters.removeAll(best_filter);
	    name_filters.prepend(best_filter);
	}
	setNameFilters(name_filters);
    }

    // save the configuration when the dialog has been accepted
    connect(this, SIGNAL(finished(int)), this, SLOT(saveConfig()));
}

//***************************************************************************
void Kwave::FileDialog::loadConfig(const QString &section)
{
    if (!section.length()) return;
    const KConfigGroup cfg = KSharedConfig::openConfig()->group(section);
    m_config_group = section;
    if (!m_last_url.isEmpty()) {
	QUrl last_path = cfg.readEntry("last_url", m_last_url);
	if (!last_path.isEmpty()) {
	    // take last path, but user defined file name
	    QString file_name = m_last_url.fileName();
	    last_path = last_path.adjusted(QUrl::RemoveFilename);
	    m_last_url = QUrl::fromUserInput(last_path.path() + file_name);
	}
    } else {
	m_last_url = QUrl::fromUserInput(cfg.readEntry("last_url",
	    m_last_url.toDisplayString()));
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

    QString file_name = selectedUrl().fileName();
    if (!file_name.length()) return; // aborted

    // store the last URL
    m_last_url = directoryUrl();

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
