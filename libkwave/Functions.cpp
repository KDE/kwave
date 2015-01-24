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

#include <config.h>
#include <math.h>

#include <QtCore/QString>

#include <klocale.h>

#include "libkwave/Functions.h"
#include "libkwave/String.h"

//***************************************************************************
static double rect(double param)
{
    double div = param / (2 * M_PI);
    param = param - (floor(div) * (2 * M_PI));
    if (param > M_PI) return -1;
    return 1;
}

//***************************************************************************
static double sin2(double param)
{
    double y = sin(param);
    return y*y;
}

//***************************************************************************
static double sin3(double param)
{
    double y = sin(param);
    return y*y*y;
}

//***************************************************************************
static double saw(double param)
{
    double div = param / (2 * M_PI);
    param -= (floor(div) * (2 * M_PI));
    param /= M_PI;
    param -= 1;
    return param;
}

//***************************************************************************
static double sawinv(double param)
{
    double div = param / (2 * M_PI);
    param -= (floor(div) * (2 * M_PI));
    param = 2 * M_PI - param;
    param /= M_PI;
    param -= 1;
    return param;
}

//***************************************************************************
static double tri(double param)
{
    param += M_PI / 2;
    double div = param / (2 * M_PI);
    param -= (floor(div) * (2 * M_PI));
    if (param > M_PI) return (((param -M_PI) / M_PI)*2)-1;
    return (((M_PI -param) / M_PI)*2)-1;
}

//***************************************************************************
static double zero(double)
{
    return 0.0;
}

//***************************************************************************
//***************************************************************************

//***************************************************************************
void Kwave::Functions::FunctionTypesMap::fill()
{
    append(0, &sin,    _("sinus"),            _(I18N_NOOP("Sinus")));
    append(1, &rect,   _("rectangular"),      _(I18N_NOOP("Rectangular")));
    append(2, &saw,    _("sawtooth"),         _(I18N_NOOP("Sawtooth")));
    append(3, &sawinv, _("inverse_sawtooth"), _(I18N_NOOP("Inverse Sawtooth")));
    append(4, &tri,    _("triangular"),       _(I18N_NOOP("Triangular")));
    append(5, &sin2,   _("square_sinus"),     _(I18N_NOOP("Square Sinus")));
    append(6, &sin3,   _("cubic_sinus"),      _(I18N_NOOP("Cubic Sinus")));
}

//***************************************************************************
Kwave::Functions::Functions()
{
}

//***************************************************************************
Kwave::Functions::~Functions()
{
}

//***************************************************************************
QString Kwave::Functions::name(unsigned int index)
{
    Q_ASSERT(index < m_functions_map.count());
    if (index >= m_functions_map.count()) return _("Zero");
    return m_functions_map.name(index);
}

//***************************************************************************
Kwave::Functions::periodic_function_t &Kwave::Functions::function(
    unsigned int index) const
{
    Kwave::Functions::periodic_function_t *f = 0;

    Q_ASSERT(index < m_functions_map.count());
    if (index < m_functions_map.count()) f = m_functions_map.data(index);

    if (!f) return *(&zero);
    return *f;
}

//***************************************************************************
unsigned int Kwave::Functions::count() const
{
    return m_functions_map.count();
}

//***************************************************************************
//***************************************************************************
