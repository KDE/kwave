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

#include <QtAlgorithms>

#include "Parser.h"
#include "Interpolation.h"
#include "Curve.h"

//***************************************************************************
QPointF Curve::NoPoint(FP_NAN, FP_NAN);

//***************************************************************************
Curve::Curve()
    :m_interpolation(INTPOL_LINEAR)
{
}

//***************************************************************************
Curve::Curve(const QString &command)
    :m_interpolation(INTPOL_LINEAR)
{
    fromCommand(command);
}

//***************************************************************************
Curve::~Curve()
{
    clear();
}

//***************************************************************************
void Curve::fromCommand(const QString &command)
{
    clear();

    Parser parse(command);
    QString t = parse.firstParam();
    setInterpolationType(m_interpolation.find(t));

    qreal x, y;
    while (!parse.isDone()) {
	x = parse.toDouble();
	if (parse.isDone()) break; // half point ?
	y = parse.toDouble();
	append(Point(x, y));
    }
}

//***************************************************************************
QString Curve::getCommand()
{
    QString cmd = "curve(";
    cmd += m_interpolation.name(m_interpolation.type());

    foreach (Point p, *this) {
	QString par = ",%1,%2";
	cmd += par.arg(p.x()).arg(p.y());
    }
    cmd += ")";
    return cmd;
}

//***************************************************************************
Interpolation &Curve::interpolation()
{
    m_interpolation.prepareInterpolation(*this);
    return m_interpolation;
}

//***************************************************************************
QVector<qreal> Curve::interpolation(unsigned int points)
{
    m_interpolation.prepareInterpolation(*this);
    return m_interpolation.interpolation(*this, points);
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
void Curve::deletePoint(Point p, bool check)
{
    Iterator it(*this);
    if (!it.findNext(p)) return;
    if ((!check) || (it.hasPrevious() && it.hasNext()))
	it.remove();
}

//***************************************************************************
void Curve::secondHalf()
{
    if (isEmpty()) return;

    foreach (Point p, *this) {
	p.setX(0.5 + p.x() / 2.0);
    }

    insert(0.0, first().y());
}

//***************************************************************************
void Curve::deleteSecondPoint()
{
    if (isEmpty()) return;

    Iterator it(*this);
    while (it.hasNext()) {
	it.next();
	it.remove();
    }
}

//***************************************************************************
void Curve::insert(qreal x, qreal y)
{
    if ((x < 0.0) || (x > 1.0)) {
	qWarning("Curve::insert(%0.2f,%0.2f): out of range !",x,y);
	return;
    }

    append(Point(x, y));
    sort();
}

//***************************************************************************
void Curve::firstHalf()
{
    if (isEmpty()) return;

    foreach (Point p, *this) {
	p.setX(p.x() / 2.0);
    }
    append(Point(1.0, first().y()));
}

//****************************************************************************
void Curve::VFlip()
{
    if (isEmpty()) return;

    foreach (Point p, *this) {
	p.setY(1.0 - p.y());
    }
}

//***************************************************************************
void Curve::HFlip()
{
    if (isEmpty()) return;

    // flip all x coordinates
    foreach (Point p, *this) {
	p.setX(1.0 - p.x());
    }

    // reverse the order it the list
    sort();
}

//***************************************************************************
void Curve::scaleFit(unsigned int range)
{
    qreal min = static_cast<qreal>((double)DBL_MAX);
    qreal max = static_cast<qreal>((double)DBL_MIN);

    Interpolation interpolation(m_interpolation.type());

    QVector<qreal> y = interpolation.interpolation(*this, range);
    foreach (qreal yi, y) {
	if (yi > max) max = yi;
	if (yi < min) min = yi;
    }

    foreach (Point p, *this) {
	p.ry() -= min;
	if (max != min)
	    p.ry() /= (max - min);
	else
	    p.ry() = min;
    }

}

//***************************************************************************
Curve::Point Curve::findPoint(qreal px, qreal py, qreal tol)
{
    Point best = NoPoint;
    qreal dist;
    qreal min_dist = tol;

    foreach (Point p, *this) {
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
static bool compare_x(Curve::Point &a, Curve::Point &b)
{
    return (a.x() < b.x());
}

//***************************************************************************
void Curve::sort()
{
    qSort(begin(), end(), compare_x);
}

//***************************************************************************
//***************************************************************************
