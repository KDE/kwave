/***************************************************************************
     WindowFunction.cpp  -  Windows functions for signal processing
			     -------------------
    begin                : Feb 05 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#include <math.h>

#include "klocale.h"

#include "libkwave/TypesMap.h"
#include "libkwave/WindowFunction.h"

//***************************************************************************
//***************************************************************************

window_function_t &operator++(window_function_t &f)
{
    return (f = (f == WINDOW_FUNC_TRIANGULAR) ?
	WINDOW_FUNC_NONE : window_function_t(f+1) );
}

//***************************************************************************
//***************************************************************************

WindowFunction::InitializedTypesMap::InitializedTypesMap()
{
    debug("WindowFunction::InitializedTypesMap::InitializedTypesMap()"); // ###
    append(WINDOW_FUNC_NONE,       WINDOW_FUNC_NONE,
           "none", "None");          i18n("None");
    append(WINDOW_FUNC_HAMMING,    WINDOW_FUNC_HAMMING,
        "hamming", "Hamming");       i18n("Hamming");
    append(WINDOW_FUNC_HANNING,    WINDOW_FUNC_HANNING,
        "hanning",  "Hanning");      i18n("Hanning");
    append(WINDOW_FUNC_BLACKMAN,   WINDOW_FUNC_BLACKMAN,
        "blackman",  "Blackman");    i18n("Blackman");
    append(WINDOW_FUNC_TRIANGULAR, WINDOW_FUNC_TRIANGULAR,
        "triangular", "Triangular"); i18n("Triangular");
}

//***************************************************************************
// static initializer
WindowFunction::InitializedTypesMap WindowFunction::m_types_map;

//***************************************************************************
//***************************************************************************
WindowFunction::WindowFunction(window_function_t type)
    :m_type(type)
{
}

//***************************************************************************
WindowFunction::~WindowFunction()
{
}

//***************************************************************************
QArray<double> WindowFunction::points(unsigned int len)
{
    QArray<double> out(len);
    ASSERT(out.count() == len);
    if (out.count() != len) {
	out.resize(0);
	return out;
    }

    // Hanning, Hamming, Blackman window functions as proposed
    // in Oppenheim, Schafer, p.241 ff
    switch (m_type) {
	case WINDOW_FUNC_NONE:   //rectangular window
	    for (unsigned int i = 0; i < len; i++)
		out[i] = 1;
	    break;
	case WINDOW_FUNC_HANNING:
	    for (unsigned int i = 0; i < len; i++)
		out[i] = 0.5 * (1 - cos(i * 2 * M_PI / (len - 1)));
	    break;
	case WINDOW_FUNC_HAMMING:
	    for (unsigned int i = 0; i < len; i++)
		out[i] = 0.54-(0.46 * cos(((double)i) * 2 * M_PI / (len - 1)));
	    break;
	case WINDOW_FUNC_BLACKMAN:
	    for (unsigned int i = 0; i < len; i++)
		out[i] = 0.42-(0.50 * cos(((double)i) * 2 * M_PI / (len - 1))) +
		              (0.08 * cos(((double)i) * 4 * M_PI / (len - 1)));
	    break;
	case WINDOW_FUNC_TRIANGULAR:
	    for (unsigned int i = 0; i < len / 2; i++)
		out[i] = ((double)i) / (len / 2 - 1);
		
	    for (unsigned int i = len / 2; i < len; i++)
		out[i] = 1 - ((double)i - len / 2) / (len / 2 - 1);
	    break;
    }

    return out;
}

//***************************************************************************
//***************************************************************************
