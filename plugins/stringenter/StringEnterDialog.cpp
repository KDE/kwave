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

#include <KConfig>
#include <KConfigGroup>
#include <KToolInvocation>

#include "libkwave/String.h"

#include "StringEnterDialog.h"

/** group for loading/saving the configuration */
#define CONFIG_GROUP metaObject()->className()

/** entry for loading/saving the width in the configuration */
#define CONFIG_WIDTH "width"

//***************************************************************************
Kwave::StringEnterDialog::StringEnterDialog(QWidget *parent,
                                            const QString &preset)
    :QDialog(parent), Ui::StringEnterDlg(), m_command()
{
    setupUi(this);
    setFixedHeight(sizeHint().height());
    setMaximumWidth(sizeHint().width() * 2);

    // restore the window width from the previous invocation
    KConfigGroup cfg    = KGlobal::config()->group(CONFIG_GROUP);
    QString      result = cfg.readEntry(CONFIG_WIDTH);
    bool         ok     = false;
    int          w      = result.toUInt(&ok);
    if (ok && (w > sizeHint().width()))
	resize(w, height());

    if (preset.length()) {
	edCommand->setText(preset);
	m_command = preset;
    }
}

//***************************************************************************
Kwave::StringEnterDialog::~StringEnterDialog()
{
    // save the window width for the next invocation
    KConfigGroup cfg = KGlobal::config()->group(CONFIG_GROUP);
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
    m_command = edCommand->userText().trimmed();
    if (m_command.length())
	QDialog::accept();
    else
	QDialog::close();
}

//***************************************************************************
void Kwave::StringEnterDialog::invokeHelp()
{
    KToolInvocation::invokeHelp(_("plugin_sect_stringenter"));
}

//***************************************************************************
//***************************************************************************
//***************************************************************************
#include "StringEnterDialog.moc"

