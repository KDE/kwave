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

#include "config.h"
#include <math.h>
#include <klocale.h>

#include "libkwave/TypesMap.h"
#include "libkwave/WindowFunction.h"

//***************************************************************************
//***************************************************************************

void WindowFunction::InitializedTypesMap::fill()
{
    append(WINDOW_FUNC_NONE,       WINDOW_FUNC_NONE,
           "none", "None");
    append(WINDOW_FUNC_HAMMING,    WINDOW_FUNC_HAMMING,
        "hamming", "Hamming");
    append(WINDOW_FUNC_HANNING,    WINDOW_FUNC_HANNING,
        "hanning",  "Hanning");
    append(WINDOW_FUNC_BLACKMAN,   WINDOW_FUNC_BLACKMAN,
        "blackman",  "Blackman");
    append(WINDOW_FUNC_TRIANGULAR, WINDOW_FUNC_TRIANGULAR,
        "triangular", "Triangular");

#undef NEVER_COMPILE_THIS
#ifdef NEVER_COMPILE_THIS
#error "this could produce problems in plugins and/or libs when \
        loaded before the main application is up."
    i18n("None");
    i18n("Hamming");
    i18n("Hanning");
    i18n("Blackman");
    i18n("Triangular");
#endif
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
QVector<qreal> WindowFunction::points(unsigned int len)
{
    QVector<qreal> out(len);
    Q_ASSERT(out.count() == static_cast<int>(len));
    if (out.count() != static_cast<int>(len)) {
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
		out[i] = 0.54-(0.46 * cos(((qreal)i) * 2 * M_PI / (len - 1)));
	    break;
	case WINDOW_FUNC_BLACKMAN:
	    for (unsigned int i = 0; i < len; i++)
		out[i] = 0.42-(0.50 * cos(((qreal)i) * 2 * M_PI / (len - 1))) +
		              (0.08 * cos(((qreal)i) * 4 * M_PI / (len - 1)));
	    break;
	case WINDOW_FUNC_TRIANGULAR:
	    for (unsigned int i = 0; i < len / 2; i++)
		out[i] = ((qreal)i) / (len / 2 - 1);

	    for (unsigned int i = len / 2; i < len; i++)
		out[i] = 1 - ((qreal)i - len / 2) / (len / 2 - 1);
	    break;
    }

    return out;
}

//***************************************************************************
//***************************************************************************
