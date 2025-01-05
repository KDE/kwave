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
#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KUrlRequester>

#include "libkwave/CodecManager.h"
#include "libkwave/FileInfo.h"
#include "libkwave/MessageBox.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "SaveBlocksDialog.h"

#define CFG_GROUP _("KwaveFileDialog-kwave_save_blocks")

//***************************************************************************
Kwave::SaveBlocksDialog::SaveBlocksDialog(QWidget *parent,
        QUrl filename,
        QString filename_pattern,
        Kwave::SaveBlocksPlugin::numbering_mode_t numbering_mode,
        bool selection_only,
        bool have_selection)
    :QDialog(parent),
     Ui::SaveBlocksDialogBase(),
     m_filename(filename.toLocalFile())
{
    setupUi(this);

    KConfigGroup cfg = KSharedConfig::openConfig()->group(CFG_GROUP);
    QUrl last_url = Kwave::URLfromUserInput(cfg.readEntry("last_url", filename.toDisplayString()));
    QString last_ext = cfg.readEntry("last_ext", _(""));

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

    // populate the extension combo box
    const QStringList encodings = Kwave::CodecManager::encodingFilter().split(_("\n"));
    for (auto e : encodings) {
        QStringList pattern;
        if (e.contains(_("|"))) {
            qsizetype i = e.indexOf(_("|"));
            pattern = e.left(i).split(_(" "));
        }
        for (auto p : pattern) {
            QString ext = p.mid(1);
            if (!cbExtension->contains(ext)) cbExtension->addItem(ext);
        }
    }
    if (!last_ext.isEmpty() && cbExtension->contains(last_ext)) {
        cbExtension->setCurrentItem(last_ext);
    }

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

    urlRequester->setUrl(last_url);

    // combo box with pattern
    connect(cbPattern, &QComboBox::editTextChanged,
            this, &SaveBlocksDialog::emitUpdate);
    connect(cbPattern, &QComboBox::highlighted,
            this, &SaveBlocksDialog::emitUpdate);
    connect(cbPattern, &QComboBox::activated,
            this, &SaveBlocksDialog::emitUpdate);

    // combo box with numbering
    connect(cbNumbering, &QComboBox::editTextChanged,
            this, &SaveBlocksDialog::emitUpdate);
    connect(cbNumbering, &QComboBox::highlighted,
            this, &SaveBlocksDialog::emitUpdate);
    connect(cbNumbering, &QComboBox::activated,
            this, &SaveBlocksDialog::emitUpdate);

    // combo box with extension
    connect(cbExtension, &QComboBox::editTextChanged,
            this, &SaveBlocksDialog::emitUpdate);
    connect(cbExtension, &QComboBox::highlighted,
            this, &SaveBlocksDialog::emitUpdate);
    connect(cbExtension, &QComboBox::activated,
            this, &SaveBlocksDialog::emitUpdate);

    // selection only checkbox
    connect(chkSelectionOnly, &QCheckBox::checkStateChanged,
            this, &SaveBlocksDialog::emitUpdate);


    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

//***************************************************************************
Kwave::SaveBlocksDialog::~SaveBlocksDialog()
{
}

//***************************************************************************
QString Kwave::SaveBlocksDialog::pattern()
{
    Q_ASSERT(cbPattern);
    return (cbPattern) ? cbPattern->currentText() : _("");
}

//***************************************************************************
Kwave::SaveBlocksPlugin::numbering_mode_t
    Kwave::SaveBlocksDialog::numberingMode()
{
    Q_ASSERT(cbNumbering);
    return (cbNumbering) ?
        static_cast<Kwave::SaveBlocksPlugin::numbering_mode_t>(
        cbNumbering->currentIndex()) : Kwave::SaveBlocksPlugin::CONTINUE;
}

//***************************************************************************
QString Kwave::SaveBlocksDialog::extension()
{
    Q_ASSERT(cbExtension);
    return (cbExtension) ? cbExtension->currentText() : _(".wav");
}

//***************************************************************************
bool Kwave::SaveBlocksDialog::selectionOnly()
{
    Q_ASSERT(chkSelectionOnly);
    return (chkSelectionOnly) ? chkSelectionOnly->isChecked() : false;
}

//***************************************************************************
QUrl Kwave::SaveBlocksDialog::selectedUrl() const
{
    QUrl url = urlRequester->url();
    if (url.isValid()) {
        return url;
    }
    return QUrl();
}

//***************************************************************************
void Kwave::SaveBlocksDialog::accept()
{
    QUrl dir = selectedUrl();
    if (dir.isValid()) {
        KConfigGroup cfg = KSharedConfig::openConfig()->group(CFG_GROUP);
        cfg.writeEntry("last_url", dir);
        cfg.writeEntry("last_ext", cbExtension->currentText());
        cfg.sync();
        QDialog::accept();
    } else {
        MessageBox::error(this, i18n("Please choose where to save the files"));
    }
}


//***************************************************************************
void Kwave::SaveBlocksDialog::setNewExample(const QString &example)
{
    Q_ASSERT(txtExample);
    if (txtExample) txtExample->setText(example);
}

void Kwave::SaveBlocksDialog::emitUpdate()
{
    Q_EMIT sigSelectionChanged(m_filename, pattern(), numberingMode(), extension(), selectionOnly());
}

//***************************************************************************
//***************************************************************************

#include "moc_SaveBlocksDialog.cpp"
