/***************************************************************************
        LowPassFilter.h  - transmission function of a low pass filter
			     -------------------
    begin                : Mar 15 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#ifndef _LOW_PASS_FILTER_H_
#define _LOW_PASS_FILTER_H_

#include "config.h"
#include "math.h"
#include <qobject.h>

#include "libkwave/TransmissionFunction.h"
#include "c_filter_stuff.h"

class LowPassFilter: public TransmissionFunction
{
    Q_OBJECT
public:

    /** Constructor */
    LowPassFilter();

    /** Destructor */
    virtual ~LowPassFilter();

    /** @see TransmissionFunction::at() */
    virtual double at(double f);

public slots:

    /** set the new cutoff frequency [0..PI] */
    void setFrequency(double f);

private:

    /** cutoff frequency [0...PI] */
    double m_f_cutoff;

    /** structure with the filter coefficients, from aRts */
    filter m_filter;

};

#endif /* _LOW_PASS_FILTER_H_ */
