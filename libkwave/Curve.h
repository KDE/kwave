/***************************************************************************
                  Curve.h  -  curve consisting of points
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

#ifndef CURVE_H
#define CURVE_H

#include "config.h"

#include <QList>
#include <QListIterator>
#include <QMutableListIterator>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include <TODO:kdemacros.h>

#include "libkwave/Interpolation.h"

namespace Kwave
{
    class Q_DECL_EXPORT Curve: public QList<QPointF>
    {
    public:

	/** Iterator */
	typedef QListIterator<QPointF> ConstIterator;

	/** Iterator */
	typedef QMutableListIterator<QPointF> Iterator;

	/** class used for the points */
	typedef QPointF Point;

	/** used for the "invalid" point */
	static QPointF NoPoint;

	/**
	 * Default constructor, creates an empty curve.
	 */
	Curve();

	/**
	 * Constructor, creates a curve from a command string.
	 * @param command string with parameters
	 */
	explicit Curve(const QString &command);

	/** Destructor */
	virtual ~Curve();

	/** Moves all current points into the left half */
	void firstHalf();

	/** Moves all current points into the right half */
	void secondHalf();

	/**
	 * Removes and deletes a point from the curve. Note that after this
	 * call the passed point is no longer valid!
	 * @param p point to be deleted
	 * @param check if true, the last or first point will not be deleted
	 */
	void deletePoint(Point p, bool check);

	/**
	 * Deletes every second point.
	 */
	void deleteSecondPoint();

	/**
	 * Flips/mirrors the curve horizontally (x-axis).
	 */
	void HFlip();

	/**
	 * Flips/mirrors the curve vertically (y-axis).
	 */
	void VFlip();

	/**
	 * Scales the curve vertically to fit into a range of (+/- range/2)
	 * on the y-axis.
	 * @param range the size range to use for scaling
	 */
	void scaleFit(unsigned int range = 1024);

	/**
	 * Creates a new point and inserts it into the curve. The new
	 * point will be sorted in by it's x coordinate.
	 * @param x coordinate on the x axis, should be [0...+1.0]
	 * @param y coordinate on the y axis, should be [0...+1.0]
	 */
	void insert(double x, double y);

	/**
	 * Searches for a point at given coordinates with a definable
	 * tolerance.
	 * @param x coordinate on the x axis
	 * @param y coordinate on the y axis
	 * @param tol tolerance for x and y direction, absolute value
	 * @return pointer to the found point or "NoPoint" if nothing found.
	 */
	Point findPoint(double x, double y, double tol = .05);

	/**
	 * Sets a curve from a command string. Opposite of getCommand().
	 * @param command a string that contains the interpolation
	 *        type and pairs of x/y coordinates.
	 */
	void fromCommand(const QString &command);

	/**
	 * Returns a command string out of the curve points and
	 * interpolation type.
	 */
	QString getCommand();

	/**
	 * Returns the interpolation type.
	 */
	Kwave::interpolation_t interpolationType();

	/**
	 * Sets a new interpolation type.
	 * @param type the new interpolation type
	 */
	void setInterpolationType(Kwave::interpolation_t type);

	/** Returns a reference to the Interpolation object itself */
	Kwave::Interpolation &interpolation();

	/**
	 * Returns an array of points, calculated out of the
	 * current interpolation parameters.
	 * @param points number of points
	 * @return Array of interpolated values or null if the
	 *         number of points was zero or the curve was empty.
	 */
	QVector<double> interpolation(unsigned int points);

    protected:

	/** sorts the list by ascending x coordinate */
	void sort();

    private:

	/** interpolation object */
	Kwave::Interpolation m_interpolation;

    };
}

#endif /* CURVE_H */

//***************************************************************************
//***************************************************************************
