/***************************************************************************
  SelectRangeDialog.cpp  -  dialog for selecting a range of samples
                             -------------------
    begin                : Sat Jun 15 2002
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
#include <math.h>

#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>

#include <KHelpClient>
#include <KLocalizedString>

#include "libkwave/String.h"

#include "SelectRangeDialog.h"

//***************************************************************************
Kwave::SelectRangeDialog::SelectRangeDialog(QWidget *widget,
    Mode start_mode, Mode range_mode, unsigned int range, double sample_rate,
    sample_index_t offset, sample_index_t signal_length)
    :QDialog(widget), Ui::SelectRangeDlg()
{
    setupUi(this);
    setModal(true);

    if (select_start) {
        select_start->init(Kwave::SelectTimeWidget::bySamples, offset,
                           sample_rate, 0, signal_length);
        select_start->setTitle(i18n("Start"));
        select_start->setMode(start_mode);
    }

    if (select_range) select_range->init(
        range_mode, range, sample_rate, offset, signal_length);

    connect(select_start, SIGNAL(valueChanged(sample_index_t)),
            select_range, SLOT(setOffset(sample_index_t)));

    setMinimumSize(sizeHint());
    setFixedSize(sizeHint());

    connect(btHelp->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this,   SLOT(invokeHelp()));

    // set the focus onto the "OK" button
    buttonBox->button(QDialogButtonBox::Ok)->setFocus();
}

//***************************************************************************
Kwave::SelectRangeDialog::~SelectRangeDialog()
{
}

//***************************************************************************
void Kwave::SelectRangeDialog::setMode(Kwave::SelectTimeWidget::Mode new_mode)
{
    if (select_range) select_range->setMode(new_mode);
}


//***************************************************************************
void Kwave::SelectRangeDialog::invokeHelp()
{
    KHelpClient::invokeHelp(_("plugin_sect_selectrange"));
}

//***************************************************************************
//***************************************************************************

#include "moc_SelectRangeDialog.cpp"
