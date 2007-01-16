/***************************************************************************
               Filter.h  -  parameters of a digital IIR or FIR filter
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

#ifndef _FILTER_H_
#define _FILTER_H_

#include "config.h"

class QString;

/**
 * @class Filter
 * Holds a set of parameters for a digital IIR or FIR filter.
 *
 * @todo use KIONetAccess in the load/save methods
 * @todo more error checks in load/save (current code is too optimistic)
 */
class Filter
{
public:
    /**
     * Constructor, creates an empty filter with a given sample rate.
     * @param rate number of samples per second
     */
    Filter(int rate);

    /**
     * Constructor, creates a filter from a Kwave command string.
     * @command part of the Kwave command with parameters
     */
    Filter(const QString &command);

    /** Destructor */
    virtual ~Filter();

    /**
     * Returns the Kwave command string from the
     * current parameters.
     */
    QString command();

    /**
     * Resizes the filter to a new number of coefficients.
     * @param newnum new number of coefficients [1..]
     * @return new number of coefficients or zero if failed
     */
    unsigned int resize(unsigned int newnum);

    /** Returns true if the filter is a FIR one, or false else */
    inline bool isFIR() { return m_fir; };

    /** Returns the sample rate in samples/second */
    inline int rate() {	return m_rate; };

    /**
     * Returns the number of coefficients and
     * delay times (order) of the filter.
     */
    unsigned int count();

    /**
     * Returns a filter coefficient.
     * @param index internal index [0...count-1]
     */
    double coeff(unsigned int index);

    /**
     * Sets a filter coefficient to a new value.
     * @param index internal index [0...count-1]
     * @param newval new coefficient
     */
    void setCoeff(unsigned int index, double newval);

    /**
     * Returns a delay time of the filter.
     * @param index internal index [0...count-1]
     */
    unsigned int delay(unsigned int index);

    /**
     * Sets a delay value to a new value.
     * @param index internal index [0...count-1]
     * @param newval new delay value
     */
    void setDelay(unsigned int index, unsigned int newval);

    /** Loads the filter parameters from a URL */
    void load(const QString &filename);

    /** Saves the filter parameters to a URL */
    void save(const QString &filename);

private:
    /** boolean if filter is FIR or IIR */
    bool m_fir;

    /** sample rate in samples/second */
    unsigned int m_rate;

    /** array of coefficients */
    QMemArray<double> m_coeff;

    /** array of delay times */
    QMemArray<int> m_delay;

};

#endif /* _FILTER_H_ */

//*****************************************************************************
//*****************************************************************************
