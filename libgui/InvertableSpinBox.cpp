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

#include <QtCore/QObject>
#include <QtGui/QWidget>

#include "libgui/InvertableSpinBox.h"

//***************************************************************************
Kwave::InvertableSpinBox::InvertableSpinBox(QWidget *parent)
    :QSpinBox(parent), m_inverse(false)
{
    connect(this, SIGNAL(valueChanged(int)),
            this, SLOT(checkValueChange(int)));
}

//***************************************************************************
void Kwave::InvertableSpinBox::setInverse(bool inverse)
{
    if (m_inverse == inverse) return; // nothing to do
    m_inverse = inverse;

    if (/* now */ inverse) {
	// relax limits by 1
	setMinimum(minimum() - 1);
	setMaximum(maximum() + 1);
    } else {
	// reduce limits by 1
	setMinimum(minimum() + 1);
	setMaximum(maximum() - 1);
    }
}

//***************************************************************************
void Kwave::InvertableSpinBox::checkValueChange(int value)
{
    if (m_inverse) {
	// in this case the real limits are tighter by 1
	if (value <= minimum()) setValue(minimum() + 1);
	if (value >= maximum()) setValue(maximum() - 1);
    }
}

//***************************************************************************
void Kwave::InvertableSpinBox::stepUp()
{
    if (m_inverse) {
	if (value() > minimum() + 1) QSpinBox::stepDown();
    } else {
	QSpinBox::stepUp();
    }
}

//***************************************************************************
void Kwave::InvertableSpinBox::stepDown()
{
    if (m_inverse) {
	if (value() + 1 < maximum()) QSpinBox::stepUp();
    } else {
	QSpinBox::stepDown();
    }
}

//***************************************************************************
#include "InvertableSpinBox.moc"
//***************************************************************************
//***************************************************************************
