/*************************************************************************
            Indexer.cpp  -  add an index to a stream
                             -------------------
    begin                : Sat Oct 16 2010
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

#include "libkwave/modules/Indexer.h"

//***************************************************************************
Kwave::Indexer::Indexer(unsigned int index)
    :Kwave::StreamObject(), m_index(index)
{
}

//***************************************************************************
Kwave::Indexer::~Indexer()
{
}

//***************************************************************************
void Kwave::Indexer::input(Kwave::SampleArray data)
{
    emit output(m_index, data);
}

//***************************************************************************
//***************************************************************************
