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
Functions::Functions()
:m_func(), m_name()
{
    append(sin,    i18n("Sinus"));
    append(rect,   i18n("Rectangular"));
    append(saw,    i18n("Sawtooth"));
    append(sawinv, i18n("Inverse Sawtooth"));
    append(tri,    i18n("Triangular"));
    append(sin2,   i18n("Square Sinus"));
    append(sin3,   i18n("Cubic Sinus"));
    i18n("Zero");
}

//***************************************************************************
Functions::~Functions()
{
    m_func.clear();
    m_name.clear();
}

//***************************************************************************
void Functions::append(periodic_function_t *func, const QString &name)
{
    periodic_function_t **f = new (periodic_function_t *);
    ASSERT(f);
    if (!f) return;

    *f = func;
    m_name.append(name);
    m_func.append(f);
}

//***************************************************************************
QString Functions::name(unsigned int index)
{
    ASSERT(index < m_name.count());
    if (index >= m_name.count()) return "Zero";
    return m_name[index];
}

//***************************************************************************
periodic_function_t &Functions::function(unsigned int index)
{
    periodic_function_t **f = 0;

    ASSERT(index < m_func.count());
    if (index < m_func.count()) f = m_func.at(index);

    if (!f || !*f) return *(&zero);
    return **f;
}

//***************************************************************************
unsigned int Functions::count()
{
    ASSERT(m_func.count() == m_name.count());
    return m_func.count();
}

//***************************************************************************
//***************************************************************************
