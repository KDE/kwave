/***************************************************************************
  InvertableSpinBox.cpp  -  a spinbox with invertable spin buttons
                             -------------------
    begin                : Sun Jan 12 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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
#include <qobject.h>
#include <qwidget.h>
#include "InvertableSpinBox.h"

//***************************************************************************
InvertableSpinBox::InvertableSpinBox(QWidget *parent, const char *name)
    :QSpinBox(parent, name), m_inverse(false)
{
    connect(this, SIGNAL(valueChanged(int)),
            this, SLOT(checkValueChange(int)));
}

//***************************************************************************
void InvertableSpinBox::setInverse(bool inverse)
{
    if (m_inverse == inverse) return; // nothing to do
    m_inverse = inverse;

    if (/* now */ inverse) {
	// relax limits by 1
	setMinValue(minValue() - 1);
	setMaxValue(maxValue() + 1);
    } else {
	// reduce limits by 1
	setMinValue(minValue() + 1);
	setMaxValue(maxValue() - 1);
    }
}

//***************************************************************************
void InvertableSpinBox::checkValueChange(int value)
{
    if (m_inverse) {
	// in this case the real limits are tighter by 1
	if (value <= minValue()) setValue(minValue()+1);
	if (value >= maxValue()) setValue(maxValue()-1);
    }
}

//***************************************************************************
void InvertableSpinBox::stepUp()
{
    if (m_inverse) {
	if (value() > minValue()+1) QSpinBox::stepDown();
    } else {
	QSpinBox::stepUp();
    }
}

//***************************************************************************
void InvertableSpinBox::stepDown()
{
    if (m_inverse) {
	if (value() < maxValue()-1) QSpinBox::stepUp();
    } else {
	QSpinBox::stepDown();
    }
}

//***************************************************************************
//***************************************************************************
