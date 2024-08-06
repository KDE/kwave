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

#include <QtGlobal>

#include "libkwave/Utils.h"

#include "BitrateSpinBox.h"

/***************************************************************************/
Kwave::BitrateSpinBox::BitrateSpinBox(QWidget *parent)
    :QSpinBox(parent), m_rates()
{
    m_rates.append(0); // don't let it stay empty, that makes life easier

    connect(this, SIGNAL(valueChanged(int)),
            this, SLOT(snapIn(int)));
}

/***************************************************************************/
Kwave::BitrateSpinBox::~BitrateSpinBox()
{
}

/***************************************************************************/
void Kwave::BitrateSpinBox::snapIn(int value)
{
    int index     = nearestIndex(value);
    int old_index = index;
    int old_value = m_rates[index];

    if (value == old_value) return;

    if ((value > old_value) && (index < Kwave::toInt(m_rates.size()) - 1))
        index++;

    if ((value < old_value) && (index > 0))
        index--;

    if (index != old_index) {
        int v = m_rates[index];
        setValue(v);
        emit snappedIn(v);
    }
}

/***************************************************************************/
void Kwave::BitrateSpinBox::allowRates(const QList<int> &list)
{
    int old_value = value();

    m_rates = list;
    if (m_rates.isEmpty()) m_rates.append(0);

    // set new ranges
    setMinimum(m_rates.first());
    setMaximum(m_rates.last());

    setValue(old_value);
}

//***************************************************************************
int Kwave::BitrateSpinBox::nearestIndex(int rate)
{
    // find the nearest value
    int nearest = 0;
    foreach(int i, m_rates)
        if (qAbs(i - rate) < qAbs(nearest - rate)) nearest = i;

    // find the index
    int index = m_rates.contains(nearest) ? m_rates.indexOf(nearest) : 0;

    // limit the index into a reasonable range
    if (index < 0)
        index = 0;
    if (index >= Kwave::toInt(m_rates.size()))
        index = m_rates.size()-1;

    return index;
}

//***************************************************************************
//***************************************************************************

#include "moc_BitrateSpinBox.cpp"
