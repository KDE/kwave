/*************************************************************************
    NoiseGenerator.cpp  -  simple noise generator, implemented as SampleSource
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
#include <stdlib.h>

#include "libkwave/Sample.h"
#include "NoiseGenerator.h"

#define BLOCK_SIZE 4096

//***************************************************************************
NoiseGenerator::NoiseGenerator(QObject *parent)
    :Kwave::SampleSource(parent, "noise generator"),
    m_noise(BLOCK_SIZE)
{
}

//***************************************************************************
NoiseGenerator::~NoiseGenerator()
{
}

//***************************************************************************
void NoiseGenerator::goOn()
{
    const unsigned int size = m_noise.size();
    sample_t *p = m_noise.data();
    float scale = 2.0 / (float)RAND_MAX;

    for (unsigned int i=0; i < size; i++, p++)
	*p = float2sample(((float)rand() * scale)) - SAMPLE_MAX;

    emit output(m_noise);
}

//***************************************************************************
#include "NoiseGenerator.moc"
//***************************************************************************
//***************************************************************************
