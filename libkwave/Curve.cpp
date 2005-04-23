/***************************************************************************
              Curve.cpp  -  curve consisting of points
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
#include <math.h>

#include <qptrlist.h>
#include <qstring.h>

#include "Parser.h"
#include "Interpolation.h"
#include "Curve.h"

//***************************************************************************
Curve::Curve()
:m_points(),
 m_interpolation(INTPOL_LINEAR)
{
    m_points.setAutoDelete(true);
}

//***************************************************************************
Curve::Curve(const QString &command)
:m_points(),
 m_interpolation(INTPOL_LINEAR)
{
    m_points.setAutoDelete(true);
    fromCommand(command);
}

//***************************************************************************
void Curve::fromCommand(const QString &command)
{
    m_points.setAutoDelete(true);
    m_points.clear();

    Parser parse(command);
    QString t = parse.firstParam();
    setInterpolationType(m_interpolation.find(t));

    double x, y;
    while (!parse.isDone()) {
	x = parse.toDouble();
	if (parse.isDone()) break; // half point ?
	y = parse.toDouble();
	append(x, y);
    }
}

//***************************************************************************
QString Curve::getCommand()
{
    QString cmd = "curve(";
    cmd += m_interpolation.name(m_interpolation.type());

    Point *p;
    for (p = m_points.first(); (p); p = m_points.next() ) {
	QString par;
	cmd += par.sprintf(",%f,%f", p->x, p->y);
    }
    cmd += ")";
    return cmd;
}

//***************************************************************************
Interpolation &Curve::interpolation()
{
    m_interpolation.prepareInterpolation(this);
    return m_interpolation;
};

//***************************************************************************
QMemArray<double> Curve::interpolation(unsigned int points)
{
    m_interpolation.prepareInterpolation(this);
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
    return m_interpolation.type();
}

//***************************************************************************
void Curve::deletePoint(Point *p, bool check)
{
    if (!p) return;
    m_points.setAutoDelete(true);
    if ((!check) || ((p != m_points.first()) && (p != m_points.last())) ) {
	m_points.remove(p);
    }
}

//***************************************************************************
void Curve::secondHalf()
{
    Point *tmp;
    if (m_points.isEmpty()) return;

    for ( tmp = m_points.first(); (tmp); tmp = m_points.next() ) {
	tmp->x = 0.5 + tmp->x / 2.0;
    }

    insert(0.0, m_points.first()->y);
}

//***************************************************************************
void Curve::deleteSecondPoint()
{
    Point *tmp;

    m_points.setAutoDelete(true);
    for ( tmp = m_points.first(); (tmp); tmp = m_points.next() ) {
	tmp = m_points.next();
	if (tmp && (tmp != m_points.last())) {
	    // m_points should have autodelete...
	    // no delete for object is required
	    m_points.removeRef(tmp);
	    tmp = m_points.prev();
	}
    }
}

//***************************************************************************
void Curve::insert(double x, double y)
{
    if ((x < 0.0) || (x > 1.0)) {
	qWarning("Curve::insert(%0.2f,%0.2f): out of range !",x,y);
	return;
    }

    if (m_points.isEmpty()) {
	// add the first point
	append(x, y);
	return;
    }

    Point *ins = new Point;
    Q_ASSERT(ins);
    if (!ins) return;
    ins->x = x;
    ins->y = y;

    // linear search for position
    Point *tmp = m_points.first();
    while ((tmp) && tmp->x < x) tmp = m_points.next();

    if (tmp) {
	// insert before some other point
	m_points.insert(m_points.at(), ins);
    } else {
	// append to the end of the list
	m_points.append(ins);
    }
}

//***************************************************************************
Curve::Point *Curve::previous(Curve::Point *act)
{
    m_points.findRef(act);
    return m_points.prev();
}

//***************************************************************************
Curve::Point *Curve::next(Curve::Point *act)
{
    m_points.findRef(act);
    return m_points.next();
}

//***************************************************************************
void Curve::append(double x, double y)
{
    Point *insert = new Point;
    Q_ASSERT(insert);
    if (!insert) return;

    insert->x = x;
    insert->y = y;
    m_points.append(insert);
}

//***************************************************************************
void Curve::firstHalf()
{
    if (m_points.isEmpty()) return;

    Point *tmp;
    for (tmp = m_points.first(); (tmp); tmp = m_points.next() ) {
	tmp->x /= 2.0;
    }
    append(1.0, m_points.first()->y);
}

//****************************************************************************
void Curve::VFlip()
{
    Point *p;
    for (p = m_points.first(); (p); p = m_points.next() ) {
	p->y = (1.0 - p->y);
    }
}

//***************************************************************************
void Curve::HFlip()
{
    // flip all x coordinates and reverse the order it the list
    unsigned int count = m_points.count();
    m_points.setAutoDelete(false);
    while (count--) {
	Point *p = m_points.at(count);
	p->x = 1.0 - p->x;
	m_points.removeRef(p);
	m_points.append(p);
    }

}

//***************************************************************************
void Curve::scaleFit(unsigned int range)
{
    Point *p;
    double min = DBL_MAX;
    double max = DBL_MIN;

    Interpolation interpolation(m_interpolation.type());

    QMemArray<double> y = interpolation.interpolation(this, range);
    for (unsigned int i = 0; i < range; i++) {
	if (y[i] > max) max = y[i];
	if (y[i] < min) min = y[i];
    }

    for (p = m_points.first(); (p); p = m_points.next()) {
	p->y -= min;
	if (max != min) p->y /= (max-min);
	else p->y=min;
    }

}

//***************************************************************************
Curve::Point *Curve::findPoint(double px, double py, double tol)
{
    Point *tmp;
    Point *best=0;
    double dist;
    double min_dist = tol;

    for ( tmp = m_points.first(); tmp; tmp = m_points.next() ) {
	// use the length of the difference vector as criterium
	dist = hypot(px - tmp->x, py - tmp->y);
	if (dist < min_dist) {
	    min_dist = dist;
	    best = tmp;
	}
    }
    return best;
}

//***************************************************************************
Curve::Point *Curve::first()
{
    return m_points.first();
}

//***************************************************************************
unsigned int Curve::count()
{
    return m_points.count();
}

//***************************************************************************
Curve::Point *Curve::at(int x)
{
    return m_points.at(x);
}

//***************************************************************************
Curve::Point *Curve::last()
{
    return m_points.last();
}

//***************************************************************************
Curve::~Curve()
{
    m_points.setAutoDelete(true);
    m_points.clear();
}

//***************************************************************************
//***************************************************************************
