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
#include <stdlib.h>
#include "MouseMark.h"

//****************************************************************************
MouseMark::MouseMark()
{
    initial = -1;
    last = -1;
}

//****************************************************************************
void MouseMark::set(int l, int r)
{
    initial = l;
    last = r;
}

//****************************************************************************
int MouseMark::left()
{
    return (initial < last) ? initial : last;
}

//****************************************************************************
int MouseMark::right()
{
    return (initial > last) ? initial : last;
}

//****************************************************************************
void MouseMark::grep(int x)
{
    if (abs((last - x)) > abs((initial - x))) {
	initial = last;
    }
    last = x;
}

//****************************************************************************
void MouseMark::update(int x)
{
    last = x;
}

//****************************************************************************
bool MouseMark::checkPosition(int x, int tol)
{
    if ((x > initial - (tol)) && (x < initial + tol))
	return true;

    if ((x < last + tol) && (x > last - tol))
	return true;

    return false;
}

//****************************************************************************
//****************************************************************************
