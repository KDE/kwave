// SPDX-FileCopyrightText: 2017 Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
// SPDX-FileCopyrightText: 2024 Mark Penner <mrp@markpenner.space>
// SPDX-License-Identifier: GPL-2.0-or-later
/***************************************************************************
 * K3BExportOptionsDialog.cpp -  dialog for K3b export options
 *                             -------------------
 *    begin                : Thu Apr 13 2017
 *    copyright            : (C) 2017 by Thomas Eschenbacher
 *    email                : Thomas.Eschenbacher@gmx.de
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
#include <libkwave/MessageBox.h>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLineEdit>

#include <KComboBox>

#include "K3BExportDialog.h"

using namespace Qt::StringLiterals;

//***************************************************************************
Kwave::K3BExportDialog::K3BExportDialog(
    QWidget *parent,
    QString &pattern,
    bool selection_only,
    bool have_selection,
    Kwave::K3BExportPlugin::overwrite_policy_t overwrite_policy,
    QUrl url
)
    :QDialog(parent), Ui::K3BExportDialogBase()
{
    setupUi(this);

    cbLabelPattern->addItem(i18nc(
        "default entry of the list of placeholder patterns in "
        "the K3b export plugin (used for detecting title and artist "
        "from a label description)",
        "(auto detect)"
    ));
    foreach (const QString &p, Kwave::K3BExportPlugin::knownPatterns())
        cbLabelPattern->addItem(p);

    Q_ASSERT(cbLabelPattern);
    if (pattern.trimmed().length())
        cbLabelPattern->setCurrentText(pattern.trimmed());
    else
        cbLabelPattern->setCurrentIndex(0);

    // the "selection only" checkbox
    Q_ASSERT(chkSelectionOnly);
    if (have_selection) {
        // we have a selection
        chkSelectionOnly->setEnabled(true);
        chkSelectionOnly->setChecked(selection_only);
    } else {
        // no selection -> force it to "off"
        chkSelectionOnly->setEnabled(false);
        chkSelectionOnly->setChecked(false);
    }

    fileUrlRequester->setStartDir(url);
    fileUrlRequester->setNameFilter(
        i18nc("file type filter when exporting to K3b", "K3b project file (*.k3b)")
    );

    dirUrlRequester->setStartDir(url);

    Q_ASSERT(cbOverwritePolicy);
    cbOverwritePolicy->setCurrentIndex(static_cast<int>(overwrite_policy));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

//***************************************************************************
Kwave::K3BExportDialog::~K3BExportDialog()
{
}

//***************************************************************************
QString Kwave::K3BExportDialog::pattern() const
{
    Q_ASSERT(cbLabelPattern);
    if (!cbLabelPattern) return QString();

    // special handling: the first entry in the list is the default pattern
    // (which is "auto-detect") -> map this to empty pattern
    QString p = cbLabelPattern->currentText().trimmed();
    if (p == cbLabelPattern->itemText(0)) return QString();

    return p;
}

//***************************************************************************
bool Kwave::K3BExportDialog::selectionOnly() const
{
    Q_ASSERT(chkSelectionOnly);
    return (chkSelectionOnly) ? chkSelectionOnly->isChecked() : false;
}

//***************************************************************************
Kwave::K3BExportPlugin::overwrite_policy_t
    Kwave::K3BExportDialog::overwritePolicy() const
{
    Q_ASSERT(cbOverwritePolicy);
    return static_cast<Kwave::K3BExportPlugin::overwrite_policy_t>(
        (cbOverwritePolicy) ?
        cbOverwritePolicy->currentIndex() : 0
    );
}

QUrl Kwave::K3BExportDialog::projectFile() const
{
    QUrl url = fileUrlRequester->url();
    if (url.isValid()) {
        return url;
    }
    return QUrl();
}

QUrl Kwave::K3BExportDialog::exportLocation() const
{
    QUrl url = dirUrlRequester->url();
    if (url.isValid()) {
        return url;
    }
    return QUrl();
}

void Kwave::K3BExportDialog::accept()
{
    QString msg;
    if (!projectFile().isValid()) {
        msg += i18n("Please choose a K3b project file name");
        msg += "\n"_L1;
    }
    if (!exportLocation().isValid()) {
        msg += i18n("Please choose where to export the audio files");
    }
    if (msg.isEmpty()) {
        QDialog::accept();
    } else {
        MessageBox::error(this, msg);
    }
}

//***************************************************************************
//***************************************************************************

#include "moc_K3BExportDialog.cpp"
