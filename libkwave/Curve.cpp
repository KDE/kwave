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

#include <float.h> // for FLT_MAX, DBL_MIN, DBL_MAX
#include <math.h>

#include <QtCore/QtAlgorithms>

#include "libkwave/Curve.h"
#include "libkwave/Interpolation.h"
#include "libkwave/Parser.h"
#include "libkwave/String.h"

//***************************************************************************

QPointF Kwave::Curve::NoPoint(FLT_MAX, FLT_MAX);

//***************************************************************************
Kwave::Curve::Curve()
    :m_interpolation(Kwave::INTPOL_LINEAR)
{
}

//***************************************************************************
Kwave::Curve::Curve(const QString &command)
    :m_interpolation(Kwave::INTPOL_LINEAR)
{
    fromCommand(command);
}

//***************************************************************************
Kwave::Curve::~Curve()
{
    clear();
}

//***************************************************************************
void Kwave::Curve::fromCommand(const QString &command)
{
    clear();

    Kwave::Parser parse(command);
    QString t = parse.firstParam();
    setInterpolationType(m_interpolation.find(t));

    double x, y;
    while (!parse.isDone()) {
	x = parse.toDouble();
	if (parse.isDone()) break; // half point ?
	y = parse.toDouble();
	append(Point(x, y));
    }
}

//***************************************************************************
QString Kwave::Curve::getCommand()
{
    QString cmd = _("curve(");
    cmd += m_interpolation.name(m_interpolation.type());

    foreach (const Point &p, *this) {
	QString par = _(",%1,%2");
	cmd += par.arg(p.x()).arg(p.y());
    }
    cmd += _(")");
    return cmd;
}

//***************************************************************************
Kwave::Interpolation &Kwave::Curve::interpolation()
{
    m_interpolation.prepareInterpolation(*this);
    return m_interpolation;
}

//***************************************************************************
QVector<double> Kwave::Curve::interpolation(unsigned int points)
{
    m_interpolation.prepareInterpolation(*this);
    return m_interpolation.interpolation(*this, points);
}

//***************************************************************************
void Kwave::Curve::setInterpolationType(Kwave::interpolation_t type)
{
    m_interpolation.setType(type);
}

//***************************************************************************
Kwave::interpolation_t Kwave::Curve::interpolationType()
{
    return m_interpolation.type();
}

//***************************************************************************
void Kwave::Curve::deletePoint(Point p, bool check)
{
    Iterator it(*this);
    if (!it.findNext(p)) return;
    if ((!check) || (it.hasPrevious() && it.hasNext()))
	it.remove();
}

//***************************************************************************
void Kwave::Curve::secondHalf()
{
    if (isEmpty()) return;

    QMutableListIterator<Point> it(*this);
    while (it.hasNext()) {
	Point &p = it.next();
	p.setX(0.5 + p.x() / 2.0);
    }

    insert(0.0, first().y());
}

//***************************************************************************
void Kwave::Curve::deleteSecondPoint()
{
    if (isEmpty()) return;

    Iterator it(*this);
    while (it.hasNext()) {
	it.next();
	it.remove();
    }
}

//***************************************************************************
void Kwave::Curve::insert(double x, double y)
{
    if ((x < 0.0) || (x > 1.0)) {
	qWarning("Curve::insert(%0.2f,%0.2f): out of range !",x,y);
	return;
    }

    append(Point(x, y));
    sort();
}

//***************************************************************************
void Kwave::Curve::firstHalf()
{
    if (isEmpty()) return;

    QMutableListIterator<Point> it(*this);
    while (it.hasNext()) {
	Point &p = it.next();
	p.setX(p.x() / 2.0);
    }
    append(Point(1.0, first().y()));
}

//****************************************************************************
void Kwave::Curve::VFlip()
{
    if (isEmpty()) return;

    QMutableListIterator<Point> it(*this);
    while (it.hasNext()) {
	Point &p = it.next();
	p.setY(1.0 - p.y());
    }
}

//***************************************************************************
void Kwave::Curve::HFlip()
{
    if (isEmpty()) return;

    // flip all x coordinates
    QMutableListIterator<Point> it(*this);
    while (it.hasNext()) {
	Point &p = it.next();
	p.setX(1.0 - p.x());
    }

    // reverse the order it the list
    sort();
}

//***************************************************************************
void Kwave::Curve::scaleFit(unsigned int range)
{
    double min = DBL_MAX;
    double max = DBL_MIN;

    Kwave::Interpolation interpolation(m_interpolation.type());

    QVector<double> y = interpolation.interpolation(*this, range);
    foreach (double yi, y) {
	if (yi > max) max = yi;
	if (yi < min) min = yi;
    }

    QMutableListIterator<Point> it(*this);
    while (it.hasNext()) {
	Point &p = it.next();
	p.ry() -= min;
	if (!qFuzzyCompare(max, min))
	    p.ry() /= (max - min);
	else
	    p.ry() = min;
    }

}

//***************************************************************************
Kwave::Curve::Point Kwave::Curve::findPoint(double px, double py, double tol)
{
    Point best = NoPoint;
    double dist;
    double min_dist = tol;

    QMutableListIterator<Point> it(*this);
    while (it.hasNext()) {
	Point &p = it.next();
	// use the length of the difference vector as criterium
	dist = hypot(px - p.x(), py - p.y());
	if (dist < min_dist) {
	    min_dist = dist;
	    best = p;
	}
    }
    return best;
}

//***************************************************************************
static bool compare_x(Kwave::Curve::Point &a, Kwave::Curve::Point &b)
{
    return (a.x() < b.x());
}

//***************************************************************************
void Kwave::Curve::sort()
{
    qSort(begin(), end(), compare_x);
}

//***************************************************************************
//***************************************************************************
