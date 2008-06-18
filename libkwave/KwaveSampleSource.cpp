/*************************************************************************
    KwaveSampleSource.cpp -  base class with a generic sample source
                             -------------------
    begin                : Sun Oct 07 2007
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
#include "libkwave/KwaveSampleSource.h"

//***************************************************************************
Kwave::SampleSource::SampleSource(QObject *parent)
    :Kwave::StreamObject(parent)
{
}

//***************************************************************************
Kwave::SampleSource::~SampleSource()
{
}

//***************************************************************************
#include "KwaveSampleSource.moc"
//***************************************************************************
//***************************************************************************
