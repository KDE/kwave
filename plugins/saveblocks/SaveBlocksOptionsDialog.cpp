// SPDX-FileCopyrightText: 2007 Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
// SPDX-FileCopyrightText: 2024 Mark Penner <mrp@markpenner.space>
// SPDX-License-Identifier: GPL-2.0-or-later
/***************************************************************************
   SaveBlocksOptionsDialog.cpp  -  dialog for extra options for saving blocks
                             -------------------
    begin                : Fri Mar 02 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
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

#include <QCheckBox>
#include <QLineEdit>

#include <KComboBox>

#include "libkwave/FileInfo.h"
#include "libkwave/String.h"

#include "SaveBlocksOptionsDialog.h"

//***************************************************************************
Kwave::SaveBlocksOptionsDialog::SaveBlocksOptionsDialog(QWidget *parent,
        QString filename,
        QString filename_pattern,
        Kwave::SaveBlocksPlugin::numbering_mode_t numbering_mode,
        bool selection_only,
        bool have_selection)
    :QDialog(parent), Ui::SaveBlocksOptionsDialogBase(), m_filename(filename)
{
    setupUi(this);

    Kwave::FileInfo info;

    // the file name pattern combo box
    cbPattern->addItem(_("[%2nr]-[%title]"));
    cbPattern->addItem(_("[%filename] part [%nr] of [%total]"));
    cbPattern->addItem(
        _("[%fileinfo{") +
        info.name(Kwave::INF_NAME) +
        _("}] (part [%nr] of [%total])"));
    cbPattern->addItem(_("[%filename] - [%04nr]"));
    cbPattern->addItem(_("[%2nr] [%filename]"));
    cbPattern->addItem(_("[%2nr]-[%filename]"));
    cbPattern->addItem(_("[%02nr]-[%filename]"));
    cbPattern->addItem(_("[%04nr]-[%filename]"));
    cbPattern->addItem(_("[%02nr] of [%count] [%filename]"));
    cbPattern->addItem(_("[%02nr] of [%total] [%filename]"));
    if (filename_pattern.length())
        cbPattern->setEditText(filename_pattern);
    else
        cbPattern->setCurrentIndex(0);

    // the numbering mode combo box
    cbNumbering->setCurrentIndex(static_cast<int>(numbering_mode));

    // the "selection only" checkbox
    if (have_selection) {
        // we have a selection
        chkSelectionOnly->setEnabled(true);
        chkSelectionOnly->setChecked(selection_only);
    } else {
        // no selection -> force it to "off"
        chkSelectionOnly->setEnabled(false);
        chkSelectionOnly->setChecked(false);
    }

    // combo box with pattern
    connect(cbPattern, &QComboBox::editTextChanged,
            this, &SaveBlocksOptionsDialog::somethingChanged);
    connect(cbPattern, &QComboBox::highlighted,
            this, &SaveBlocksOptionsDialog::somethingChanged);
    connect(cbPattern, &QComboBox::activated,
            this, &SaveBlocksOptionsDialog::somethingChanged);

    // combo box with numbering
    connect(cbNumbering, &QComboBox::editTextChanged,
            this, &SaveBlocksOptionsDialog::somethingChanged);
    connect(cbNumbering, &QComboBox::highlighted,
            this, &SaveBlocksOptionsDialog::somethingChanged);
    connect(cbNumbering, &QComboBox::activated,
            this, &SaveBlocksOptionsDialog::somethingChanged);

    // selection only checkbox
    connect(chkSelectionOnly, &QCheckBox::checkStateChanged,
            this, &SaveBlocksOptionsDialog::somethingChanged);

    connect(this, &SaveBlocksOptionsDialog::somethingChanged,
            this, &SaveBlocksOptionsDialog::emitUpdate);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

//***************************************************************************
Kwave::SaveBlocksOptionsDialog::~SaveBlocksOptionsDialog()
{
}

//***************************************************************************
QString Kwave::SaveBlocksOptionsDialog::pattern()
{
    Q_ASSERT(cbPattern);
    return (cbPattern) ? cbPattern->currentText() : _("");
}

//***************************************************************************
Kwave::SaveBlocksPlugin::numbering_mode_t
    Kwave::SaveBlocksOptionsDialog::numberingMode()
{
    Q_ASSERT(cbNumbering);
    return (cbNumbering) ?
        static_cast<Kwave::SaveBlocksPlugin::numbering_mode_t>(
        cbNumbering->currentIndex()) : Kwave::SaveBlocksPlugin::CONTINUE;
}

//***************************************************************************
bool Kwave::SaveBlocksOptionsDialog::selectionOnly()
{
    Q_ASSERT(chkSelectionOnly);
    return (chkSelectionOnly) ? chkSelectionOnly->isChecked() : false;
}

//***************************************************************************
void Kwave::SaveBlocksOptionsDialog::setNewExample(const QString &example)
{
    Q_ASSERT(txtExample);
    if (txtExample) txtExample->setText(example);
}

void Kwave::SaveBlocksOptionsDialog::emitUpdate()
{
    Q_EMIT sigSelectionChanged(m_filename, pattern(), numberingMode(), selectionOnly());
}

//***************************************************************************
//***************************************************************************

#include "moc_SaveBlocksOptionsDialog.cpp"
