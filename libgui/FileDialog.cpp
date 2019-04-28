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
#include <QMimeDatabase>
#include <QMimeType>
#include <QPushButton>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QUrl>

#include <KConfig>
#include <KConfigGroup>
#include <KFile>
#include <KFileFilterCombo>
#include <KSharedConfig>

#include "libkwave/CodecManager.h"
#include "libkwave/String.h"

#include "libgui/FileDialog.h"

//***************************************************************************
Kwave::FileDialog::FileDialog(
    const QString &startDir,
    OperationMode mode,
    const QString &filter,
    QWidget *parent,
    const QUrl last_url,
    const QString last_ext
)
    :QDialog(parent),
     m_layout(this),
     m_file_widget(QUrl::fromUserInput(startDir), this),
     m_config_group(),
     m_last_url(last_url),
     m_last_ext(last_ext)
{
    const bool saving = (mode == SaveFile);

    // do some layout and init stuff
    m_layout.addWidget(&m_file_widget);
    setMinimumSize(m_file_widget.dialogSizeHint());
    setModal(true);
    connect(&m_file_widget, SIGNAL(filterChanged(QString)),
            this, SIGNAL(filterChanged(QString)));

    // connect the Cancel button
    QPushButton *button;
    button = m_file_widget.cancelButton();
    connect(button, SIGNAL(clicked()), this, SLOT(reject()));
    button->show();

    // connect the Open/Save button
    button = m_file_widget.okButton();
    connect(&m_file_widget, SIGNAL(accepted()), this, SLOT(accept()));
    connect(button, SIGNAL(clicked(bool)), &m_file_widget, SLOT(slotOk()));
    button->show();

    switch (mode) {
	case SaveFile:
	    m_file_widget.setOperationMode(KFileWidget::Saving);
	    m_file_widget.setMode(KFile::File);
	    m_file_widget.setConfirmOverwrite(false);
	    break;
	case OpenFile:
	    m_file_widget.setOperationMode(KFileWidget::Opening);
	    m_file_widget.setMode(KFile::File |
	                          KFile::ExistingOnly);
	    break;
	case SelectDir:
	    m_file_widget.setOperationMode(KFileWidget::Opening);
	    m_file_widget.setMode(KFile::Directory |
	                          KFile::ExistingOnly);
	    break;
    }

    QString special_prefix = _("kfiledialog:///");
    if (startDir.startsWith(special_prefix)) {
	// configuration key given
	m_file_widget.setKeepLocation(true);

	// load initial settings
	QString section = startDir;
	section = section.remove(0, special_prefix.length());
	if (section.contains(_("/")))
	    section = section.left(section.indexOf(_("/")));
	section.prepend(_("KwaveFileDialog-"));
	loadConfig(section);
    }

    // if a file extension was passed but no filter, try to guess
    // the mime type
    QString file_filter = filter;
    if (!file_filter.length() && m_last_ext.length()) {
	file_filter = guessFilterFromFileExt(m_last_ext, mode);
	qDebug("guessed filter for '%s': '%s",
	       DBG(m_last_ext), DBG(file_filter));
    }

    // if a filename or directory URL was passed, try to re-use it
    if (!m_last_url.isEmpty() && (m_last_url.isLocalFile() || saving)) {
	QFileInfo file(m_last_url.toLocalFile());
	if (QFileInfo::exists(file.path()) || saving)
	    m_file_widget.setUrl(m_last_url.adjusted(QUrl::RemoveFilename));
	if (!file.isDir() && (file.exists() || saving))
	    m_file_widget.setSelectedUrl(
		QUrl::fromLocalFile(m_last_url.fileName()));
    }

    // parse the list of file filters and put the last extension on top
    if (file_filter.length()) {
	QStringList filter_list = file_filter.split(_("\n"));
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

	    // put the last extension to the top of the list
	    // and thus make it selected
	    if (m_last_ext.length()) {
		QStringList extensions = p.split(_(" "));
		if (extensions.contains(m_last_ext)) {
		    if (!best_filter.length() ||
			(p.length() <= best_pattern.length())) {
			best_filter  = filter_item;
			best_pattern = p;
		    }
		}
	    }

	    name_filters.append(filter_item);
	}
	if (best_filter.length()) {
	    name_filters.removeAll(best_filter);
	    name_filters.prepend(best_filter);
	}

	m_file_widget.setFilter(name_filters.join(QChar::fromLatin1('\n')));
    }
}

//***************************************************************************
void Kwave::FileDialog::loadConfig(const QString &section)
{
    if (!section.length()) return;
    KConfigGroup cfg = KSharedConfig::openConfig()->group(section);
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

    // get last dialog size (Kwave global config)
    cfg = KSharedConfig::openConfig()->group("FileDialog");
    int w = cfg.readEntry("dialog_width",  sizeHint().width());
    int h = cfg.readEntry("dialog_height", sizeHint().height());
    if (w < minimumWidth())  w = sizeHint().width();
    if (h < minimumHeight()) w = sizeHint().height();
    resize(w, h);
}

//***************************************************************************
void Kwave::FileDialog::saveConfig()
{
    if (!m_config_group.length()) return;
    if (selectedUrl().isEmpty()) return;

    QString file_name = selectedUrl().fileName();
    if (!file_name.length()) return; // aborted

    // store the last URL
    m_last_url = m_file_widget.baseUrl();

    // store the last extension if present
    QFileInfo file(file_name);
    QString extension = file.suffix();
    if (extension.length()) {
	// simple case: file extension
	m_last_ext = _("*.") + extension;
    } else {
	// tricky case: filename mask
	QString pattern = m_file_widget.currentFilter();
	if (pattern.contains(_("|"))) {
	    int i = pattern.indexOf(_("|"));
	    pattern = pattern.left(i);
	}
	m_last_ext = _("");
	foreach (const QString &mask, pattern.split(_(" "))) {
	    QRegExp regex(mask, Qt::CaseSensitive, QRegExp::Wildcard);
	    if (regex.indexIn(file_name) >= 0) {
		m_last_ext = mask;
		break;
	    }
	}
    }

    KConfigGroup cfg = KSharedConfig::openConfig()->group(m_config_group);
    cfg.writeEntry("last_url",      m_last_url);
    cfg.writeEntry("last_ext",      m_last_ext);
    cfg.sync();

    // save the geometry of the dialog (Kwave global config)
    cfg = KSharedConfig::openConfig()->group("FileDialog");
    cfg.writeEntry("dialog_width",  width());
    cfg.writeEntry("dialog_height", height());
    cfg.sync();
}

//***************************************************************************
QString Kwave::FileDialog::selectedExtension()
{
    QStringList ext_list = m_file_widget.currentFilter().split(_(" "));
    return *(ext_list.begin());
}

//***************************************************************************
QUrl Kwave::FileDialog::selectedUrl() const
{
    return m_file_widget.selectedUrl();
}

//***************************************************************************
QUrl Kwave::FileDialog::baseUrl() const
{
    return m_file_widget.baseUrl();
}

//***************************************************************************
void Kwave::FileDialog::setDirectory(const QString &directory)
{
    m_file_widget.setStartDir(QUrl::fromUserInput(directory));
}

//***************************************************************************
void Kwave::FileDialog::selectUrl(const QUrl &url)
{
    m_file_widget.setUrl(url);
}

//***************************************************************************
void Kwave::FileDialog::accept()
{
    m_file_widget.accept();

    // save the configuration when the dialog was accepted
    saveConfig();

    QDialog::accept();
}

//***************************************************************************
QString Kwave::FileDialog::guessFilterFromFileExt(const QString &pattern,
                                                  OperationMode mode)
{
    // if there are multiple extensions in a list, iterate over them
    if (pattern.contains(_(" "))) {
	QStringList patterns = pattern.split(_(" "));
	foreach (const QString &p, patterns) {
	    QString f = guessFilterFromFileExt(p, mode);
	    if (f.length()) return f;
	}
    }

    // get the filter list of our own codecs, this prefers the Kwave internal
    // mime types against the ones from KDE
    QString filters = (mode == SaveFile) ?
	Kwave::CodecManager::encodingFilter() :
	Kwave::CodecManager::decodingFilter();
    foreach (const QString &filter, filters.split(_(" "))) {
	QString p = filter;
	if (p.contains(_("|"))) {
	    int i = p.indexOf(_("|"));
	    p = p.left(i);
	}
	foreach (const QString &mask, p.split(_(" "))) {
	    if (mask == pattern) {
// 		qDebug("MATCH from CodecManager: '%s' matches '%s' -> '%s'",
// 		       DBG(mask), DBG(pattern), DBG(filter));
		return filter;
	    }
	}
    }

    // try to find out by asking the Qt mime database
    QMimeDatabase db;
    QList<QMimeType> mime_types = db.mimeTypesForFileName(pattern);
    if (!mime_types.isEmpty()) {
	foreach (const QMimeType &m, mime_types) {
	    if (m.isValid() && !m.isDefault()) {
		QString filter_string =
		    m.globPatterns().join(_(" ")) + _("|") +
		    m.filterString();
// 		qDebug("MATCH from QMimeDatabase: '%s' -> '%s'",
// 		       DBG(pattern), DBG(filter_string));
		return filter_string;
	    }
	}
    }

    return QString();
}

//***************************************************************************
void Kwave::FileDialog::setCustomWidget(QWidget *widget)
{
    m_file_widget.setCustomWidget(widget);
}

//***************************************************************************
KUrlComboBox *Kwave::FileDialog::locationEdit() const
{
    return m_file_widget.locationEdit();
}

//***************************************************************************
//***************************************************************************
