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

#include <math.h>

#include <qradiobutton.h>
#include <qslider.h>

#include <klocale.h>
#include <knuminput.h>

#include "libgui/IntValidatorProxy.h"
#include "SelectRangeDialog.h"

//***************************************************************************
SelectRangeDialog::SelectRangeDialog(QWidget *widget,
    Mode mode, double range, double sample_rate,
    unsigned int offset, unsigned int signal_length)
    :SelectRangeDlg(widget, 0, true)
{
    setFixedSize(sizeHint());
    if (select_range) select_range->init(
        mode, range, sample_rate, offset, signal_length);
}

//***************************************************************************
SelectRangeDialog::~SelectRangeDialog()
{
}

//***************************************************************************
void SelectRangeDialog::setMode(SelectTimeWidget::Mode new_mode)
{
    if (select_range) select_range->setMode(new_mode);
}

//***************************************************************************
//***************************************************************************
