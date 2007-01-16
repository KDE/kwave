/***************************************************************************
   KDatePickerAdapter.h  -  adapter for using KDatePicker in a .ui file
                             -------------------
    begin                : Thu Aug 01 2002
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

#ifndef _K_DATE_PICKER_ADAPTER_H_
#define _K_DATE_PICKER_ADAPTER_H_

#include "config.h"
#include <kdatepicker.h>
class QWidget;

/**
 * @class KDatePickerAdapter
 * Adapter for making the KDatePicker widget usable in a .ui file.
 * Only defines a constructor.
 */
class KDatePickerAdapter: public KDatePicker
{
public:
    /** Constructor needed by the XML gui system */
    KDatePickerAdapter(QWidget *parent, const char *)
        :KDatePicker(parent)
    {};

    /** Destructor */
    virtual ~KDatePickerAdapter() {};

};

#endif /* _K_DATE_PICKER_ADAPTER_H_ */
