/***************************************************************************
           MouseMark.cpp -  Handling of mouse selection
			     -------------------
    begin                : Sun Nov 12 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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
#include "MouseMark.h"

//****************************************************************************
MouseMark::MouseMark()
{
    initial = 0;
    last = 0;
}

//****************************************************************************
void MouseMark::set(unsigned int l, unsigned int r)
{
    initial = l;
    last = r;
}

//****************************************************************************
unsigned int MouseMark::left()
{
    return (initial < last) ? initial : last;
}

//****************************************************************************
unsigned int MouseMark::right()
{
    return (initial > last) ? initial : last;
}

//****************************************************************************
void MouseMark::grep(unsigned int x)
{
    double d_last  = (double)last    - (double)x;
    double d_first = (double)initial - (double)x;
    d_last  *= d_last;
    d_first *= d_first;
    if (d_last > d_first) {
	initial = last;
    }
    last = x;
}

//****************************************************************************
void MouseMark::update(unsigned int x)
{
    last = x;
}

//****************************************************************************
//****************************************************************************
