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

#ifndef _CURVE_H_
#define _CURVE_H_

#include "config.h"
#include <qarray.h>
#include <qobject.h>
#include <qstring.h>

#include "Interpolation.h"
#include "PointSet.h"

class Curve: public QObject
{
public:

    /**
     * Default constructor, creates an empty curve.
     */
    Curve();

    /**
     * Constructor, creates a curve from a command string.
     * @param command string with parameters
     */
    Curve (const QString &command);

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
    void deletePoint(Point *p, bool check);

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
     * @param range
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
     * @return pointer to the found point or null if nothing found.
     */
    Point *findPoint(double x, double y, double tol = .05);

    /**
     * Returns the first point of the curve or
     * null if the curve is empty.
     */
    Point *first();

    /**
     * Returns the number of points in the curve.
     */
    unsigned int count();

    /**
     * Returns the point at a given index.
     * @param x index wthin the curve [0...count-1].
     */
    Point *at(int x);

    /**
     * Returns the last point of the curve or
     * null if the curve is empty.
     */
    Point *last();

    /**
     * Returns a point before a given point or null if the
     * given point was the first one.
     * @param act the point after the one we look for
     */
    Point *previous(Point *act);

    /**
     * Returns a point after a given point or null if the
     * given point was the first one.
     * @param act the point before the one we look for
     */
    Point *next(Point *p);


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
    interpolation_t interpolationType();

    /**
     * Sets a new interpolation type.
     * @param type the new interpolation type
     */
    void setInterpolationType(interpolation_t type);

    /** Returns a reference to the Interpolation object itself */
    Interpolation &interpolation();

    /**
     * Returns an array of points, calculated out of the
     * current interpolation parameters.
     * @param points number of points
     * @return Array of interpolated values or null if the
     *         number of points was zero or the curve was empty.
     */
    QArray<double> interpolation(unsigned int points);

protected:
    /**
     * Creates a new point and appends it to the end of the curve.
     * @param x coordinate on the x axis, should be [0...+1.0]
     * @param y coordinate on the y axis, should be [0...+1.0]
     * @note this could break sorting if used from outside this class
     */
    void append(double x, double y);

private:

    /** list of points, sorted by x coordinates */
    QList<Point> m_points;

    /** interpolation object */
    Interpolation m_interpolation;

};

#endif /* _CURVE_H_ */
