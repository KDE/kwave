/***************************************************************************
  StringEnterDialog.cpp  -  dialog for entering a string command
                             -------------------
    begin                : Sat Mar 14 2015
    copyright            : (C) 2015 by Thomas Eschenbacher
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

#include <QComboBox>

#include <KConfig>
#include <KConfigGroup>
#include <KHelpClient>
#include <KSharedConfig>

#include "libkwave/String.h"

#include "StringEnterDialog.h"

/** group for loading/saving the configuration */
#define CONFIG_GROUP _("plugin stringenter")

/** entry for loading/saving the width in the configuration */
#define CONFIG_WIDTH "width"

/** maximum number of entries in the history */
#define CONFIG_MAX_HISTORY 20

//***************************************************************************
Kwave::StringEnterDialog::StringEnterDialog(QWidget *parent,
                                            const QString &preset)
    :QDialog(parent), Ui::StringEnterDlg(), m_command()
{
    setupUi(this);
    setFixedHeight(sizeHint().height());
    setMaximumWidth(sizeHint().width() * 2);

    // restore the window width from the previous invocation
    KConfigGroup cfg    = KSharedConfig::openConfig()->group(CONFIG_GROUP);
    QString      result = cfg.readEntry(CONFIG_WIDTH);
    bool         ok     = false;
    int          w      = result.toUInt(&ok);
    if (ok && (w > sizeHint().width()))
        resize(w, height());

    if (preset.length()) {
        cbCommand->setCurrentText(preset);
        m_command = preset;
    }

    // restore the last entries from history
    cbCommand->setMaxVisibleItems(CONFIG_MAX_HISTORY);
    cbCommand->setDuplicatesEnabled(false);
    for (int i = 0; i < CONFIG_MAX_HISTORY; i++) {
        QString entry = cfg.readEntry(QString::number(i));
        if (!entry.isEmpty()) cbCommand->addItem(entry);
    }
}

//***************************************************************************
Kwave::StringEnterDialog::~StringEnterDialog()
{
    // save the window width for the next invocation
    KConfigGroup cfg = KSharedConfig::openConfig()->group(CONFIG_GROUP);
    cfg.writeEntry(CONFIG_WIDTH, width());
}

//***************************************************************************
QString Kwave::StringEnterDialog::command()
{
    return m_command;
}

//***************************************************************************
void Kwave::StringEnterDialog::accept()
{
    m_command = cbCommand->currentText().trimmed();
    if (m_command.length()) {
        // save the entries into the history
        KConfigGroup cfg = KSharedConfig::openConfig()->group(CONFIG_GROUP);
        cfg.writeEntry(QString::number(0), m_command);
        int idx = 1;
        for (int i = 0; i < cbCommand->count(); i++) {
            QString entry = cbCommand->itemText(i).trimmed();
            if (entry == m_command) entry.clear();
            if (!entry.isEmpty()) cfg.writeEntry(QString::number(idx++), entry);
        }
        for (int i = idx; i < CONFIG_MAX_HISTORY; i++) {
            cfg.deleteEntry(QString::number(i));
        }
        QDialog::accept();
    } else {
        QDialog::close();
    }
}

//***************************************************************************
void Kwave::StringEnterDialog::invokeHelp()
{
    KHelpClient::invokeHelp(_("plugin_sect_stringenter"));
}

//***************************************************************************
//***************************************************************************
