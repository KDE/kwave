/***************************************************************************
        Interpolation.h  -  Interpolation types
			     -------------------
    begin                : Sat Feb 03 2001
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
#ifndef _INTERPOLATION_H_
#define _INTERPOLATION_H_

#include "config.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>

#include <kdemacros.h>

#include "libkwave/TypesMap.h"

namespace Kwave
{

    class Curve;

    typedef enum {
	INTPOL_LINEAR = 0,
	INTPOL_SPLINE,
	INTPOL_NPOLYNOMIAL,
	INTPOL_POLYNOMIAL3,
	INTPOL_POLYNOMIAL5,
	INTPOL_POLYNOMIAL7,
	INTPOL_SAH
    } interpolation_t;

    /**
     * Interpolation types
     */
    class KDE_EXPORT Interpolation
    {
    public:
	/** Constructor, initializes type by enum type */
	explicit Interpolation(Kwave::interpolation_t type = INTPOL_LINEAR);

	/** Destructor. */
	virtual ~Interpolation();

	bool prepareInterpolation(const Kwave::Curve &points);

	QVector<double> interpolation(const Kwave::Curve &points,
	                              unsigned int len);

	QVector<double> limitedInterpolation(const Kwave::Curve &points,
	                                     unsigned int len);

	/**
	 * Returns a single point of the interpolation.
	 */
	double singleInterpolation(double pos);

	/**
	 * Same as getSingleInterpolation, but return value
	 * will be limited to be [0...1]
	 * @see #singleInterpolation
	 * @param pos ???
	 * @return interpolated value [0...1]
	 */
	double singleLimitedInterpolation(double pos);

	/**
	 * Returns the if of a type through it's name.
	 * @param name the short name of the interpolation, like used in a command
	 * @return the interpolation
	 */
	static Kwave::interpolation_t find(const QString &name)
	{
	    return m_interpolation_map.findFromName(name);
	}

	/**
	 * Returns the name of an interpolation (non-localized).
	 * @param type the type to get the name of
	 */
	static QString name(Kwave::interpolation_t type);

	/**
	 * Returns an alphabetically sorted list of verbose
	 * interpolation type names, useful for providing a list
	 * of available types in the gui.
	 * @param localized if true, the list will contain localized
	 *        names (useful for filling combo boxes)
	 */
	static QStringList descriptions(bool localized = false);

	/** Sets a new interpolation tpye */
	inline void setType (Kwave::interpolation_t t) {
	    m_type = t;
	}

	/** Returns the currently interpolation selected type */
	inline Kwave::interpolation_t type() {
	    return m_type;
	}

	/** Translates an index in an interpolation type */
	static inline Kwave::interpolation_t findByIndex(int index) {
	    return m_interpolation_map.findFromData(index);
	}

	/**
	 * Little private class for initialized map. Used
	 * to translate interpolation_t into verbose name
	 * and vice-versa.
	 */
	class InterpolationMap:
	    public Kwave::TypesMap<Kwave::interpolation_t, int >
	{
	public:

	    /** Constructor */
	    explicit InterpolationMap()
		:TypesMap<Kwave::interpolation_t, int >()
	    {
		fill();
	    }

	    /** filling function for the map. */
	    void fill();
	};

    private:

	/** Returns the number of points **/
	unsigned int count();

	/**
	 * ???
	 * @param points curve with points for interpolation
	 * @param x receives all x coordinates ???
	 * @param y receives all y coordinates ???
	 */
	void createFullPolynom(const Kwave::Curve &points,
	                       QVector<double> &x,
	                       QVector<double> &y);

	/**
	 * ???
	 * @param x array of x coordinates
	 * @param y array of y coordinates
	 * @param ab array for return values
	 * @param n ???
	 */
	void get2Derivate(const QVector<double> &x,
	                  const QVector<double> &y,
	                  QVector<double> &ab, unsigned int n);

	/**
	 * ???
	 * @param points curve with points for interpolation
	 * @param x array of x coordinates
	 * @param y array of y coordinates
	 * @param pos ???
	 * @param degree ???
	 */
	void createPolynom (const Kwave::Curve &points,
	                    QVector<double> &x,
	                    QVector<double> &y,
	                    int pos, unsigned int degree);

    private:

	/**  List of points to be interpolated. */
	const Kwave::Curve *m_curve;

	/** ??? used for temporary purposes */
	QVector<double> m_x;

	/** ??? used for temporary purposes */
	QVector<double> m_y;

	/** ??? used for temporary purposes */
	QVector<double> m_der;

	/** Map with type and name of interpolations */
	static InterpolationMap m_interpolation_map;

	/** Type of the interpolation. */
	Kwave::interpolation_t m_type;

    };

    //***********************************************************************
    static inline Kwave::interpolation_t &operator ++(Kwave::interpolation_t &i)
    {
	return (i = (i == INTPOL_SAH) ?
	    INTPOL_LINEAR :
	    Kwave::interpolation_t(i + 1) );
    }

}

#endif /* _INTERPOLATION_H_ */

//***************************************************************************
//***************************************************************************
