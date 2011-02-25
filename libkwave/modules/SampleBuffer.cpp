/*************************************************************************
       SampleBuffer.cpp  -  simple buffer for sample arrays
                             -------------------
    begin                : Sun Oct 17 2010
    copyright            : (C) 2010 by Thomas Eschenbacher
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

#include "libkwave/modules/SampleBuffer.h"

//***************************************************************************
Kwave::SampleBuffer::SampleBuffer()
    :Kwave::StreamObject(), m_data()
{
}

//***************************************************************************
Kwave::SampleBuffer::~SampleBuffer()
{
}

//***************************************************************************
bool Kwave::SampleBuffer::isEmpty() const
{
    return m_data.isEmpty();
}

//***************************************************************************
Kwave::SampleArray &Kwave::SampleBuffer::data()
{
    return m_data;
}

//***************************************************************************
void Kwave::SampleBuffer::done()
{
    emit output(m_data);
}

//***************************************************************************
void Kwave::SampleBuffer::input(Kwave::SampleArray data)
{
    m_data = data;
}

//***************************************************************************
#include "SampleBuffer.moc"
//***************************************************************************
//***************************************************************************
