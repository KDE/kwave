/***************************************************************************
     BitrateSpinBox.cpp  -  spinbox for selecting a bitrate for MP3 or Ogg
			     -------------------
    begin                : Thu Oct 24 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
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
#include <stdlib.h> // for abs()

#include "BitrateSpinBox.h"

/***************************************************************************/
BitrateSpinBox::BitrateSpinBox(QWidget *parent, const char *name)
    :QSpinBox(parent, name), m_rates()
{
    m_rates.append(0); // don't let it stay empty, that makes life easier

    connect(this, SIGNAL(valueChanged(int)),
            this, SLOT(snapIn(int)));
}

/***************************************************************************/
BitrateSpinBox::~BitrateSpinBox()
{
}

/***************************************************************************/
void BitrateSpinBox::snapIn(int value)
{
    int old_value = value;
    int index = nearestIndex(old_value);
    int new_value = m_rates[index];
    QSpinBox::setValue(new_value);

    if (new_value != old_value) {
	emit snappedIn(new_value);
    }
}

/***************************************************************************/
void BitrateSpinBox::stepUp()
{
    int index = nearestIndex(value());
    if (index < (int)m_rates.size()-1) {
	index++;
	int value = m_rates[index];
	setValue(value);
	emit snappedIn(value);
    }
}

/***************************************************************************/
void BitrateSpinBox::stepDown()
{
    int index = nearestIndex(value());
    if (index > 0) {
	index--;
	int value = m_rates[index];
	setValue(value);
	emit snappedIn(value);
    }
}

/***************************************************************************/
void BitrateSpinBox::allowRates(const QValueList<int> &list)
{
    int old_value = value();

    m_rates.clear();
    m_rates += list;
    if (m_rates.isEmpty()) m_rates.append(0);

    // set new ranges
    setMinValue(m_rates.first());
    setMaxValue(m_rates.last());

    setValue(old_value);
}

//***************************************************************************
int BitrateSpinBox::nearestIndex(int rate)
{
    // find the nearest value
    int nearest = 0;
    QValueList<int>::iterator it;
    for (it=m_rates.begin(); it != m_rates.end(); ++it)
	if (abs(*it - rate) < abs(nearest - rate)) nearest = *it;

    // find the index
    int index = m_rates.findIndex(nearest);

    // limit the index into a reasonable range
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < (int)m_rates.size());
    if (index < 0)                    index = 0;
    if (index >= (int)m_rates.size()) index = m_rates.size()-1;

    return index;
}

//***************************************************************************
#include "BitrateSpinBox.moc"
//***************************************************************************
//***************************************************************************
