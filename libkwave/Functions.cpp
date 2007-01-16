/***************************************************************************
          Functions.cpp  -  list of simple periodic functions
			     -------------------
    begin                : Jan 21 2001
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

#include <klocale.h>
#include "Functions.h"

//***************************************************************************
double rect(double param)
{
    double div = param / (2 * M_PI);
    param = param - (floor(div) * (2 * M_PI));
    if (param > M_PI) return -1;
    return 1;
}

//***************************************************************************
double sin2(double param)
{
    double y = sin(param);
    return y*y;
}

//***************************************************************************
double sin3(double param)
{
    double y = sin(param);
    return y*y*y;
}

//***************************************************************************
double saw(double param)
{
    double div = param / (2 * M_PI);
    param -= (floor(div) * (2 * M_PI));
    param /= M_PI;
    param -= 1;
    return param;
}

//***************************************************************************
double sawinv(double param)
{
    double div = param / (2 * M_PI);
    param -= (floor(div) * (2 * M_PI));
    param = 2 * M_PI - param;
    param /= M_PI;
    param -= 1;
    return param;
}

//***************************************************************************
double tri(double param)
{
    param += M_PI / 2;
    double div = param / (2 * M_PI);
    param -= (floor(div) * (2 * M_PI));
    if (param > M_PI) return (((param -M_PI) / M_PI)*2)-1;
    return (((M_PI -param) / M_PI)*2)-1;
}

//***************************************************************************
double zero(double)
{
    return 0.0;
}

//***************************************************************************
//***************************************************************************

//***************************************************************************
void Functions::FunctionTypesMap::fill()
{
    append(0, &sin,    "sinus",            "Sinus");
    append(1, &rect,   "rectangular",      "Rectangular");
    append(2, &saw,    "sawtooth",         "Sawtooth");
    append(3, &sawinv, "inverse_sawtooth", "Inverse Sawtooth");
    append(4, &tri,    "triangular",       "Triangular");
    append(5, &sin2,   "square_sinus",     "Square Sinus");
    append(6, &sin3,   "cubic_sinus",      "Cubic Sinus");

#undef NO_NEED_TO_COMPILE_THIS
#ifdef NO_NEED_TO_COMPILE_THIS
    i18n("Sinus");
    i18n("Rectangular");
    i18n("Sawtooth");
    i18n("Inverse Sawtooth");
    i18n("Triangular");
    i18n("Square Sinus");
    i18n("Cubic Sinus");
    i18n("Zero");
#endif
}

//***************************************************************************
Functions::Functions()
{
}

//***************************************************************************
Functions::~Functions()
{
}

//***************************************************************************
QString Functions::name(unsigned int index)
{
    Q_ASSERT(index < m_functions_map.count());
    if (index >= m_functions_map.count()) return "Zero";
    return m_functions_map.name(index);
}

//***************************************************************************
periodic_function_t &Functions::function(unsigned int index)
{
    periodic_function_t *f = 0;

    Q_ASSERT(index < m_functions_map.count());
    if (index < m_functions_map.count()) f = m_functions_map.data(index);

    if (!f) return *(&zero);
    return *f;
}

//***************************************************************************
unsigned int Functions::count()
{
    return m_functions_map.count();
}

//***************************************************************************
//***************************************************************************
