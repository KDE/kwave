/***************************************************************************
        NotchFilter.h  - transmission function of a notch filter
			     -------------------
    begin                : Thu Jun 19 2003
    copyright            : (C) 2003 by Dave Flogeras
    email                : d.flogeras@unb.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _NOTCH_FILTER_H_
#define _NOTCH_FILTER_H_

#include "config.h"
#include "math.h"
#include <qobject.h>

#include "libkwave/TransmissionFunction.h"
#include "c_filter_stuff.h"

class NotchFilter: public TransmissionFunction
{
    Q_OBJECT
public:

    /** Constructor */
    NotchFilter();

    /** Destructor */
    virtual ~NotchFilter();

    /** @see TransmissionFunction::at() */
    virtual double at(double f);

public slots:

    /** set the new cutoff frequency and bw [0..PI] */
    void setFrequency(double f, double bw);

private:

    /** cutoff frequency [0...PI] */
    double m_f_cutoff;
    /** bandwidth of the notch */
    double m_f_bw;

    /** structure with the filter coefficients, from aRts */
    filter m_filter;

};

#endif /* _NOTCH_FILTER_H_ */
