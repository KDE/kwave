/***************************************************************************
   SaveBlocksWidget.cpp  -  widget for extra options in the file open dialog
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
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include "SaveBlocksWidget.h"

//***************************************************************************
SaveBlocksWidget::SaveBlocksWidget(QWidget *parent,
	QString filename_pattern,
	SaveBlocksPlugin::numbering_mode_t numbering_mode,
	bool selection_only,
	bool have_selection)
    :SaveBlocksWidgetBase(parent, 0)
{
    // the file name pattern combo box
    cbPattern->setCurrentText(filename_pattern);

    // the numbering mode combo box
    cbNumbering->setCurrentItem(static_cast<int>(numbering_mode));

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
    connect(cbPattern, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    connect(cbPattern, SIGNAL(highlighted(int)),
            this, SLOT(indexChanged(int)));
    connect(cbPattern, SIGNAL(activated(int)),
            this, SLOT(indexChanged(int)));

    // combo box with numbering
    connect(cbNumbering, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    connect(cbNumbering, SIGNAL(highlighted(int)),
            this, SLOT(indexChanged(int)));
    connect(cbNumbering, SIGNAL(activated(int)),
            this, SLOT(indexChanged(int)));

    // selection only checkbox
    connect(chkSelectionOnly, SIGNAL(stateChanged(int)),
	    this, SLOT(indexChanged(int)));

    emit somethingChanged();
}

//***************************************************************************
SaveBlocksWidget::~SaveBlocksWidget()
{
}

//***************************************************************************
QString SaveBlocksWidget::pattern()
{
    Q_ASSERT(cbPattern);
    return (cbPattern) ? cbPattern->currentText() : "";
}

//***************************************************************************
SaveBlocksPlugin::numbering_mode_t SaveBlocksWidget::numberingMode()
{
    Q_ASSERT(cbNumbering);
    return (cbNumbering) ? static_cast<SaveBlocksPlugin::numbering_mode_t>(
	cbNumbering->currentItem()) : SaveBlocksPlugin::CONTINUE;
}

//***************************************************************************
bool SaveBlocksWidget::selectionOnly()
{
    Q_ASSERT(chkSelectionOnly);
    return (chkSelectionOnly) ? chkSelectionOnly->isChecked() : false;
}

//***************************************************************************
void SaveBlocksWidget::setNewExample(const QString &example)
{
    Q_ASSERT(txtExample);
    if (txtExample) txtExample->setText(example);
}

//***************************************************************************
void SaveBlocksWidget::textChanged(const QString &)
{
    emit somethingChanged();
}

//***************************************************************************
void SaveBlocksWidget::indexChanged(int)
{
    emit somethingChanged();
}

//***************************************************************************
#include "SaveBlocksWidget.moc"
//***************************************************************************
//***************************************************************************
