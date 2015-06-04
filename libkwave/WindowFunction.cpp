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
#include <KI18n/KLocalizedString>

#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/WindowFunction.h"

//***************************************************************************
//***************************************************************************

void Kwave::WindowFunction::InitializedTypesMap::fill()
{
    append(WINDOW_FUNC_NONE,       WINDOW_FUNC_NONE,
           _("none"),       _(I18N_NOOP("None")));
    append(WINDOW_FUNC_HAMMING,    WINDOW_FUNC_HAMMING,
           _("hamming"),    _(I18N_NOOP("Hamming")));
    append(WINDOW_FUNC_HANNING,    WINDOW_FUNC_HANNING,
           _("hanning"),    _(I18N_NOOP("Hanning")));
    append(WINDOW_FUNC_BLACKMAN,   WINDOW_FUNC_BLACKMAN,
           _("blackman"),   _(I18N_NOOP("Blackman")));
    append(WINDOW_FUNC_TRIANGULAR, WINDOW_FUNC_TRIANGULAR,
           _("triangular"), _(I18N_NOOP("Triangular")));
}

//***************************************************************************
// static initializer
Kwave::WindowFunction::InitializedTypesMap Kwave::WindowFunction::m_types_map;

//***************************************************************************
//***************************************************************************
Kwave::WindowFunction::WindowFunction(Kwave::window_function_t type)
    :m_type(type)
{
}

//***************************************************************************
Kwave::WindowFunction::~WindowFunction()
{
}

//***************************************************************************
QVector<double> Kwave::WindowFunction::points(unsigned int len) const
{
    QVector<double> out(len);
    Q_ASSERT(out.count() == Kwave::toInt(len));
    if (out.count() != Kwave::toInt(len)) {
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
		out[i] = 0.54-(0.46 * cos(static_cast<double>(i) * 2 * M_PI /
		         (len - 1)));
	    break;
	case WINDOW_FUNC_BLACKMAN:
	    for (unsigned int i = 0; i < len; i++)
		out[i] = 0.42-(0.50 * cos(static_cast<double>(i) * 2 * M_PI /
		         (len - 1))) +
		         (0.08 * cos(static_cast<double>(i) * 4 * M_PI /
		         (len - 1)));
	    break;
	case WINDOW_FUNC_TRIANGULAR:
	    for (unsigned int i = 0; i < len / 2; i++)
		out[i] = static_cast<double>(i) / (len / 2 - 1);

	    for (unsigned int i = len / 2; i < len; i++)
		out[i] = 1 - (static_cast<double>(i) - len / 2) / (len / 2 - 1);
	    break;
    }

    return out;
}

//***************************************************************************
//***************************************************************************
