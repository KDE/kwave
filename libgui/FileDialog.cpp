// SPDX-FileCopyrightText: 2002 Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
// SPDX-FileCopyrightText: 2024 Mark Penner <mrp@markpenner.space>
// SPDX-License-Identifier: GPL-2.0-or-later
/*************************************************************************
    KwaveFileDialog.cpp  -  wrapper for QFileDialog
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

#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QRegularExpression>
#include <QStringList>

#include <KConfig>
#include <KConfigGroup>
#include <KFile>
#include <KFileFilterCombo>
#include <KSharedConfig>

#include "libkwave/CodecManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "libgui/FileDialog.h"

using namespace Qt::StringLiterals;

//***************************************************************************
Kwave::FileDialog::FileDialog(
    const QString &startDir,
    OperationMode mode,
    const QString &filter,
    QWidget *parent,
    const QUrl last_url,
    const QString last_ext
)
    :QObject(parent),
     m_file_dialog(new QFileDialog(parent)),
     m_config_group(),
     m_last_url(last_url),
     m_last_ext(last_ext)
{
    const bool saving = (mode == SaveFile);

    switch (mode) {
        case SaveFile:
            m_file_dialog->setAcceptMode(QFileDialog::AcceptSave);
            m_file_dialog->setFileMode(QFileDialog::AnyFile);
            break;
        case OpenFile:
            m_file_dialog->setFileMode(QFileDialog::ExistingFile);
            break;
        case SelectDir:
            m_file_dialog->setFileMode(QFileDialog::Directory);
            break;
        DEFAULT_IGNORE;
    }

    QString special_prefix = _("kfiledialog:///");
    if (startDir.startsWith(special_prefix)) {
        // load initial settings
        QString section = startDir;
        section = section.remove(0, special_prefix.length());
        if (section.contains(_("/")))
            section = section.left(section.indexOf(_("/")));
        section.prepend(_("KwaveFileDialog-"));
        loadConfig(section);
    } else {
        QUrl d{startDir};
        m_file_dialog->setDirectoryUrl(d.adjusted(QUrl::RemoveFilename));
    }

    // if a file extension was passed but no filter, try to guess
    // the mime type
    QString file_filter = filter;
    if (!file_filter.length() && m_last_ext.length()) {
        file_filter = guessFilterFromFileExt(m_last_ext, mode);
        qDebug("guessed filter for '%s': '%s'",
               DBG(m_last_ext), DBG(file_filter));
    }

    // if a filename or directory URL was passed, try to re-use it
    if (!m_last_url.isEmpty() && (m_last_url.isLocalFile() || saving)) {
        QFileInfo file(m_last_url.toLocalFile());
        if (QFileInfo::exists(file.path()) || saving)
            m_file_dialog->setDirectoryUrl(m_last_url.adjusted(QUrl::RemoveFilename));
        if (!file.isDir() && (file.exists() || saving))
            m_file_dialog->selectUrl(
                QUrl::fromLocalFile(m_last_url.fileName()));
    }

    // parse the list of file filters and put the last extension on top
    if (file_filter.length()) {
        const QStringList filter_list = file_filter.split(u"\n"_s);
        QStringList name_filters;
        QString best_filter;
        for(const QString &filter_item : filter_list) {
            QString f(filter_item);
            QString pattern(u"*"_s);
            if (f.contains(_("|"))) {
                qsizetype i = f.indexOf("|"_L1);
                pattern = f.left(i);
                f = f.mid(i + 1);
            }
            if (!f.length()) continue;

            // put filter together like QFileDialog expects it
            file_filter = f + "("_L1 + pattern + ")"_L1;

            // keep track of the last filter that contains the last used extension
            if (m_last_ext.length() && pattern.contains(m_last_ext)) {
                best_filter = file_filter;
            }
            name_filters.append(file_filter);
        }
        // put the last extension to the top of the list
        // and make it selected
        if (best_filter.length()) {
            name_filters.removeAll(best_filter);
            name_filters.prepend(best_filter);
        }

        m_file_dialog->setNameFilters(name_filters);
    }
}

//***************************************************************************
int Kwave::FileDialog::exec()
{
    int result = m_file_dialog->exec();
    if (result == QDialog::Accepted) {
        saveConfig();
    }
    return result;
}

//***************************************************************************
void Kwave::FileDialog::loadConfig(const QString &section)
{
    if (!section.length()) return;
    KConfigGroup cfg = KSharedConfig::openConfig()->group(section);
    m_config_group = section;
    if (!m_last_url.isEmpty()) {
        QUrl last_path = cfg.readEntry("last_url", m_last_url);
        if (m_file_dialog->fileMode() == QFileDialog::Directory) {
            m_last_url = Kwave::URLfromUserInput(last_path.path());
        } else {
            // take last path, but user defined file name
            QString file_name = m_last_url.fileName();
            last_path = last_path.adjusted(QUrl::RemoveFilename);
            m_last_url = Kwave::URLfromUserInput(last_path.path() + file_name);
        }
    } else {
        m_last_url = Kwave::URLfromUserInput(cfg.readEntry("last_url",
            m_last_url.toDisplayString()));
    }
    if (!m_last_ext.length())
        m_last_ext = cfg.readEntry("last_ext", m_last_ext);

    // get last dialog size (Kwave global config)
    cfg = KSharedConfig::openConfig()->group(u"FileDialog"_s);
    int w = cfg.readEntry("dialog_width", m_file_dialog->sizeHint().width());
    int h = cfg.readEntry("dialog_height", m_file_dialog->sizeHint().height());
    if (w < m_file_dialog->minimumWidth())  w = m_file_dialog->sizeHint().width();
    if (h < m_file_dialog->minimumHeight()) w = m_file_dialog->sizeHint().height();
    m_file_dialog->resize(w, h);
}

//***************************************************************************
void Kwave::FileDialog::saveConfig()
{
    if (!m_config_group.length()) return;
    if (selectedUrl().isEmpty()) return;

    QString file_name = selectedUrl().fileName();
    if (!file_name.length()) return; // aborted

    // store the last URL
    m_last_url = baseUrl();

    // store the last extension if present
    QFileInfo file(file_name);
    QString extension = file.suffix();
    if (extension.length()) {
        // simple case: file extension
        m_last_ext = _("*.") + extension;
    } else {
        // tricky case: filename mask
        QString pattern = m_file_dialog->selectedNameFilter();
        if (pattern.contains(_("|"))) {
            qsizetype i = pattern.indexOf(_("|"));
            pattern = pattern.left(i);
        }
        m_last_ext = _("");
        foreach (const QString &mask, pattern.split(_(" "))) {
            QRegularExpression regex{
                QRegularExpression::wildcardToRegularExpression(mask)};
            if (file_name.indexOf(regex) >= 0) {
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
    cfg = KSharedConfig::openConfig()->group(u"FileDialog"_s);
    cfg.writeEntry("dialog_width",  m_file_dialog->width());
    cfg.writeEntry("dialog_height", m_file_dialog->height());
    cfg.sync();
}

//***************************************************************************
QString Kwave::FileDialog::selectedExtension()
{
    QStringList patterns = m_file_dialog->selectedNameFilter().split(u" "_s);
    return (!patterns.isEmpty()) ? patterns.constFirst() : QString();
}

//***************************************************************************
QUrl Kwave::FileDialog::selectedUrl() const
{
    if(!m_file_dialog->selectedUrls().isEmpty()) {
        return m_file_dialog->selectedUrls().first();
    }
    return QUrl();
}

//***************************************************************************
QUrl Kwave::FileDialog::baseUrl() const
{
    // m_file_dialog->directoryUrl() is the obvious choice, but for some reason
    // when using the flatpak portal and when in AcceptOpen mode, it just returned
    // the directory used when it was last opened in AcceptSave mode.
    if (m_file_dialog->fileMode() == QFileDialog::Directory) {
        // when choosing a directory, QFileDialog doesn't include a slash at the end
        // so add one now
        return QUrl(selectedUrl().toString() + u"/"_s);
    }
    return selectedUrl().adjusted(QUrl::RemoveFilename);
}

//***************************************************************************
void Kwave::FileDialog::setDirectory(const QString &directory)
{
    m_file_dialog->setDirectoryUrl(Kwave::URLfromUserInput(directory));
}

//***************************************************************************
void Kwave::FileDialog::setWindowTitle(const QString &title)
{
    m_file_dialog->setWindowTitle(title);
}

//***************************************************************************
void Kwave::FileDialog::selectUrl(const QUrl &url)
{
    m_file_dialog->selectUrl(url);
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
            qsizetype i = p.indexOf(_("|"));
            p = p.left(i);
        }
        foreach (const QString &mask, p.split(_(" "))) {
            if (mask == pattern) {
//              qDebug("MATCH from CodecManager: '%s' matches '%s' -> '%s'",
//                     DBG(mask), DBG(pattern), DBG(filter));
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
//              qDebug("MATCH from QMimeDatabase: '%s' -> '%s'",
//                     DBG(pattern), DBG(filter_string));
                return filter_string;
            }
        }
    }

    return QString();
}

//***************************************************************************
//***************************************************************************

#include "moc_FileDialog.cpp"
