/***************************************************************************
              Curve.cpp  -  parameters of a curve consisting of points
			     -------------------
    begin                : Jan 20 2001
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
#include <float.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <qlist.h>
#include <qstring.h>

#include "Parser.h"
#include "PointSet.h"
#include "Interpolation.h"

#include "Curve.h"

//***************************************************************************
Curve::Curve()
:m_points(),
 m_interpolation(*(new Interpolation(INTPOL_LINEAR))),
 m_interpolationtype(INTPOL_LINEAR)
{
    m_points.setAutoDelete(true);
}

//***************************************************************************
Curve::Curve(const char *command)
:m_points(),
 m_interpolation(*(new Interpolation(INTPOL_LINEAR))),
 m_interpolationtype(INTPOL_LINEAR)
{
    m_points.setAutoDelete(true);

    Parser parse(command);
    setInterpolationType(Interpolation::find(parse.firstParam(), false));

    double x = 1.0;
    double y;
    while (!parse.isDone()) {
	x = parse.toDouble();
	y = parse.toDouble();
	append (x, y);
    }
}

//***************************************************************************
QString Curve::getCommand()
{
    QString s;
    char buf[64];
    Point *tmp;

    s = "curve (";
    s += Interpolation::name(m_interpolationtype);

    for ( tmp = m_points.first(); tmp; tmp = m_points.next() ) {
	snprintf(buf, sizeof(buf), ",%f,%f", tmp->x, tmp->y);
	s += buf;
    }

    s += ")";
    return s;
}

//***************************************************************************
QArray<double> *Curve::interpolation(unsigned int points)
{
    return m_interpolation.interpolation(this, points);
}

//***************************************************************************
void Curve::setInterpolationType(interpolation_t type)
{
    m_interpolation.setType(type);
}

//***************************************************************************
interpolation_t Curve::interpolationType()
{
    return m_interpolationtype;
}

//***************************************************************************
void Curve::deletePoint(Point *p, bool check)
{
    if ((!check) || ((p != m_points.first()) && (p != m_points.last())) ) {
	m_points.removeRef(p);
    }
}

//***************************************************************************
void Curve::secondHalf()
{
    Point *tmp;

    for ( tmp = m_points.first(); tmp; tmp = m_points.next() ) {
	ASSERT(tmp);
	if (tmp) tmp->x = 0.5 + tmp->x / 2.0;
    }

    tmp = m_points.first();
    Point *newpoint = new Point;
    ASSERT(newpoint);
    ASSERT(tmp);
    if (!newpoint) return;
    if (!tmp) return;

    newpoint->x = 0.0;
    newpoint->y = tmp->y;
    m_points.insert(0, newpoint);
}

//***************************************************************************
void Curve::deleteSecondPoint()
{
    Point *tmp;

    for ( tmp = m_points.first(); tmp; tmp = m_points.next() ) {
	tmp = m_points.next();
	if (tmp && tmp != m_points.last()) {
	    // m_points should have autodelete...
	    // no delete for object is required
	    m_points.removeRef(tmp);
	    tmp = m_points.prev();
	}
    }
}

//***************************************************************************
void Curve::addPoint(Point *insert)
{
    ASSERT(insert);
    if (insert == 0) return;

    Point *tmp = m_points.first();

    // linear search for position
    while ((tmp) && ((tmp->x) < (insert->x)) )
	tmp = m_points.next();

    if (tmp) m_points.insert(m_points.at(), insert);
    else debug ("Curve::addPoint(): point is out of range !\n");
}

//***************************************************************************
void Curve::addPoint(double x, double y)
{
    Point *insert = new Point;
    ASSERT(insert);
    if (!insert) return;

    insert->x = x;
    insert->y = y;
    addPoint (insert);
}

//***************************************************************************
Point *Curve::previous(Point *act)
{
    m_points.findRef(act);
    return m_points.prev();
}

//***************************************************************************
Point *Curve::next(Point *act)
{
    m_points.findRef(act);
    return m_points.next();
}

//***************************************************************************
void Curve::append(double x, double y)
{
    Point *insert = new Point;
    ASSERT(insert);
    if (!insert) return;

    insert->x = x;
    insert->y = y;
    append(insert);
}

//***************************************************************************
void Curve::append(Point *p)
{
    ASSERT(p);
    if (p) m_points.append(p);
}

//***************************************************************************
void Curve::firstHalf()
{
    Point *tmp;
    for ( tmp = m_points.first(); tmp; tmp = m_points.next() ) {
	tmp->x = tmp->x / 2.0;
    }

    tmp = m_points.first();
    Point *newpoint = new Point;
    ASSERT(newpoint);
    if (!newpoint) return;

    newpoint->x = 1.0;
    newpoint->y = tmp->y;
    m_points.insert (0, newpoint);
}

//****************************************************************************
void Curve::VFlip()
{
    Point *tmp;
    for ( tmp = m_points.first(); tmp != 0; tmp = m_points.next() ) {
	tmp->y = (1.0 - tmp->y);
    }
}

//***************************************************************************
void Curve::HFlip()
{
    Point *tmp;
    QList<Point> newlist;

    for (tmp = m_points.last(); tmp; tmp = m_points.prev()) {
	tmp->x = 1.0 - tmp->x;
	newlist.append (tmp);
    }
    m_points = newlist;
}

//***************************************************************************
void Curve::scaleFit(int range)
{
    struct Point *tmp;
    double min = DBL_MAX;
    double max = DBL_MIN;

    tmp = m_points.first();
    if (!tmp) return;

    Interpolation interpolation(m_interpolationtype);

    QArray<double> *y = interpolation.interpolation(this, range);
    ASSERT(y);
    if (y) {
	for (int i = 0; i < range; i++) {
	    if (*y[i] > max) max = *y[i];
	    if (*y[i] < min) min = *y[i];
	}
    }

    for (tmp = m_points.first(); tmp; tmp = m_points.next()) {
	tmp->y -= min;
	if (max!=min) tmp->y /= (max-min);
	else tmp->y=min;
    }

    if (y) delete y;
}

//***************************************************************************
Point *Curve::findPoint(double px, double py, double tol)
// checks, if given coordinates fit to a control point in the list...
{
    double maxx = px + tol;
    double minx = px - tol;
    double maxy = py + tol;
    double miny = py - tol;

    Point *tmp;
    Point *act = 0;

    for ( tmp = m_points.first(); tmp; tmp = m_points.next() ) {
	double x = tmp->x;
	double y = tmp->y;

	if (x > maxx) break;
	//the list should be sorted, a match cannot be
	//found anymore, because x is already to big

	if ((x >= minx) && (x <= maxx) && (y >= miny) && (y <= maxy)) {
	    act = tmp;
	    break;
	}
    }
    return act;
}

//***************************************************************************
Point *Curve::first()
{
    return m_points.first();
}

//***************************************************************************
unsigned int Curve::count()
{
    return m_points.count();
}

//***************************************************************************
Point *Curve::at(int x)
{
    return m_points.at(x);
}

//***************************************************************************
Point *Curve::last()
{
    return m_points.last();
}

//***************************************************************************
Curve::~Curve()
{
}

//***************************************************************************
//***************************************************************************
