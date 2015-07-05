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

#include "NoiseGenerator.h"
#include "libkwave/Sample.h"

//***************************************************************************
Kwave::NoiseGenerator::NoiseGenerator(QObject *parent)
    :Kwave::SampleSource(parent), m_buffer(blockSize()), m_noise_level(1.0)
{
}

//***************************************************************************
Kwave::NoiseGenerator::~NoiseGenerator()
{
}

//***************************************************************************
void Kwave::NoiseGenerator::goOn()
{
    emit output(m_buffer);
}

//***************************************************************************
void Kwave::NoiseGenerator::input(Kwave::SampleArray data)
{
    bool ok = m_buffer.resize(data.size());
    Q_ASSERT(ok);
    Q_UNUSED(ok);

    m_buffer = data;

    const double alpha = (1.0 - m_noise_level);
    const double scale = (m_noise_level * 2.0) / static_cast<double>(RAND_MAX);
    for (unsigned i = 0; i < data.size(); ++i) {
	const Kwave::SampleArray &in = data;
	m_buffer[i] = double2sample(
	    (sample2double(in[i]) * alpha) +
	    ((rand() - (RAND_MAX / 2)) * scale)
	);
    }
}

//***************************************************************************
void Kwave::NoiseGenerator::setNoiseLevel(const QVariant fc)
{
    m_noise_level = QVariant(fc).toDouble();
}

//***************************************************************************
//***************************************************************************
