/***************************************************************************
         KwaveDelay.cpp  -  delay line for small delays
                             -------------------
    begin                : Sun Nov 11 2007
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
#include <QString>

#include "libkwave/modules/KwaveDelay.h"

//***************************************************************************
Kwave::Delay::Delay()
    :Kwave::SampleSource(), m_fifo(), m_out_buffer(blockSize()), m_delay(0)
{
}

//***************************************************************************
Kwave::Delay::~Delay()
{
}

//***************************************************************************
void Kwave::Delay::goOn()
{
    m_fifo.get(m_out_buffer);
    emit output(m_out_buffer);
}

//***************************************************************************
void Kwave::Delay::input(Kwave::SampleArray data)
{
    m_fifo.put(data);
}

//***************************************************************************
void Kwave::Delay::setDelay(const QVariant &d)
{
    unsigned int new_delay = QVariant(d).toUInt();
    if (new_delay == m_delay) return; // nothing to do

    // fill it with zeroes, up to the delay time
    m_fifo.flush();
    Kwave::SampleArray zeroes(blockSize());
    for (unsigned int pos=0; pos < blockSize(); ++pos)
	zeroes[pos] = 0;
    unsigned int rest = new_delay;
    while (rest) {
	unsigned int len = blockSize();
	if (rest < len) zeroes.resize(rest);
	m_fifo.put(zeroes);
	rest -= zeroes.size();
    }
}

//***************************************************************************
#include "KwaveDelay.moc"
//***************************************************************************
//***************************************************************************
