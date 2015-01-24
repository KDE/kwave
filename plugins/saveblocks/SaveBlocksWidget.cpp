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

#include <config.h>

#include <QtGui/QCheckBox>

#include <kcombobox.h>
#include <klineedit.h>

#include "libkwave/String.h"

#include "SaveBlocksWidget.h"

//***************************************************************************
Kwave::SaveBlocksWidget::SaveBlocksWidget(QWidget *parent,
	QString filename_pattern,
	Kwave::SaveBlocksPlugin::numbering_mode_t numbering_mode,
	bool selection_only,
	bool have_selection)
    :QWidget(parent), Ui::SaveBlocksWidgetBase()
{
    setupUi(this);

    // the file name pattern combo box
    cbPattern->addItem(_("[%2nr] [%filename]"));
    cbPattern->addItem(_("[%2nr]-[%filename]"));
    cbPattern->addItem(_("[%02nr]-[%filename]"));
    cbPattern->addItem(_("[%04nr]-[%filename]"));
    cbPattern->addItem(_("[%02nr] of [%count] [%filename]"));
    cbPattern->addItem(_("[%02nr] of [%total] [%filename]"));
    cbPattern->addItem(_("[%filename] part [%nr] of [%total]"));
    cbPattern->addItem(_("[%filename] - [%04nr]"));
    cbPattern->setEditText(filename_pattern);

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
    connect(cbPattern, SIGNAL(editTextChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    connect(cbPattern, SIGNAL(highlighted(int)),
            this, SLOT(indexChanged(int)));
    connect(cbPattern, SIGNAL(activated(int)),
            this, SLOT(indexChanged(int)));

    // combo box with numbering
    connect(cbNumbering, SIGNAL(editTextChanged(const QString &)),
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
Kwave::SaveBlocksWidget::~SaveBlocksWidget()
{
}

//***************************************************************************
QString Kwave::SaveBlocksWidget::pattern()
{
    Q_ASSERT(cbPattern);
    return (cbPattern) ? cbPattern->currentText() : _("");
}

//***************************************************************************
Kwave::SaveBlocksPlugin::numbering_mode_t Kwave::SaveBlocksWidget::numberingMode()
{
    Q_ASSERT(cbNumbering);
    return (cbNumbering) ?
	static_cast<Kwave::SaveBlocksPlugin::numbering_mode_t>(
	cbNumbering->currentIndex()) : Kwave::SaveBlocksPlugin::CONTINUE;
}

//***************************************************************************
bool Kwave::SaveBlocksWidget::selectionOnly()
{
    Q_ASSERT(chkSelectionOnly);
    return (chkSelectionOnly) ? chkSelectionOnly->isChecked() : false;
}

//***************************************************************************
void Kwave::SaveBlocksWidget::setNewExample(const QString &example)
{
    Q_ASSERT(txtExample);
    if (txtExample) txtExample->setText(example);
}

//***************************************************************************
void Kwave::SaveBlocksWidget::textChanged(const QString &)
{
    emit somethingChanged();
}

//***************************************************************************
void Kwave::SaveBlocksWidget::indexChanged(int)
{
    emit somethingChanged();
}

//***************************************************************************
#include "SaveBlocksWidget.moc"
//***************************************************************************
//***************************************************************************
