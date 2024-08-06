/***************************************************************************
 * K3BExportWidget.cpp -  widget for K3b export options in the file open dlg
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

#include <QCheckBox>
#include <QLineEdit>

#include <KComboBox>

#include "libkwave/String.h"

#include "K3BExportWidget.h"

//***************************************************************************
Kwave::K3BExportWidget::K3BExportWidget(
    QWidget *parent,
    QString &pattern,
    bool selection_only,
    bool have_selection,
    Kwave::K3BExportPlugin::export_location_t export_location,
    Kwave::K3BExportPlugin::overwrite_policy_t overwrite_policy
)
    :QWidget(parent), Ui::K3BExportWidgetBase()
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

    Q_ASSERT(cbExportLocation);
    cbExportLocation->setCurrentIndex(static_cast<int>(export_location));

    Q_ASSERT(cbOverwritePolicy);
    cbOverwritePolicy->setCurrentIndex(static_cast<int>(overwrite_policy));
}

//***************************************************************************
Kwave::K3BExportWidget::~K3BExportWidget()
{
}

//***************************************************************************
QString Kwave::K3BExportWidget::pattern() const
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
bool Kwave::K3BExportWidget::selectionOnly() const
{
    Q_ASSERT(chkSelectionOnly);
    return (chkSelectionOnly) ? chkSelectionOnly->isChecked() : false;
}

//***************************************************************************
Kwave::K3BExportPlugin::export_location_t
    Kwave::K3BExportWidget::exportLocation() const
{
    Q_ASSERT(cbExportLocation);
    return static_cast<Kwave::K3BExportPlugin::export_location_t>(
        (cbExportLocation) ?
        cbExportLocation->currentIndex() : 0
    );
}

//***************************************************************************
Kwave::K3BExportPlugin::overwrite_policy_t
    Kwave::K3BExportWidget::overwritePolicy() const
{
    Q_ASSERT(cbOverwritePolicy);
    return static_cast<Kwave::K3BExportPlugin::overwrite_policy_t>(
        (cbOverwritePolicy) ?
        cbOverwritePolicy->currentIndex() : 0
    );
}

//***************************************************************************
//***************************************************************************

#include "moc_K3BExportWidget.cpp"
