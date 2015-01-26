/***************************************************************************
        MixerMatrix.cpp  -  channel mixer matrix
			     -------------------
    begin                : 2012-05-11
    copyright            : (C) 2012 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>

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

#include "libkwave/MixerMatrix.h"

//***************************************************************************
Kwave::MixerMatrix::MixerMatrix(unsigned int inputs, unsigned int outputs)
    :Kwave::Matrix<double>(inputs, outputs)
{
    for (unsigned int y = 0; y < outputs; y++) {
	unsigned int m1, m2;

	m1 = y * inputs;
	m2 = (y + 1) * inputs;

	for (unsigned int x = 0; x < inputs; x++) {
	    unsigned int n1, n2;
	    n1 =  x * outputs;
	    n2 = n1 + outputs;

	    // get the common area of [n1..n2] and [m1..m2]
	    unsigned int l = (n1 > m1) ? n1 : m1;
	    unsigned int r = (n2 < m2) ? n2 : m2;

	    (*this)[x][y] = (r > l) ? static_cast<double>(r - l) /
		static_cast<double>(inputs) : 0.0;
	}
    }
}

//***************************************************************************
Kwave::MixerMatrix::~MixerMatrix()
{
}

//***************************************************************************
//***************************************************************************
