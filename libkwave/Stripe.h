/***************************************************************************
               Stripe.h  -  continuous block of samples
			     -------------------
    begin                : Feb 09 2001
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

#ifndef _STRIPE_H_
#define _STRIPE_H_

#include <qarray.h>

#include "mt/Mutex.h"

#include "libkwave/Sample.h"

//***************************************************************************
class Stripe
{
public:

    /**
     * Default constructor. Creates an empty stripe with zero-length.
     */
    Stripe();

    /**
     * Constructor. Creates a stripe with specified length.
     * @param length number of samples
     */
    Stripe(unsigned int length);

    /**
     * Constructor. Creates a stripe that already contains samples.
     * @param samples array with initial samples
     */
    Stripe(const QArray<sample_t> &samples);

    /**
     * Returns a reference to alock for the whole stripe.
     */
    Mutex &mutex();

    /**
     * Returns the current length of the stripe in samples.
     */
    unsigned int length();

    /**
     * Resizes the stripe to a new number of samples. If the array
     * size is reduced, samples from the end are thrown away. If
     * the size is increased, samples with zero value will be added
     * to the end.
     * @param length new length of the array [samples]
     * @return new length [samples]
     */
    unsigned int resize(unsigned int length);

    /**
     * Operator for appending a number of samples to the
     * end of the stripe.
     */
    void operator << (const QArray<sample_t> &samples);

private:
    /** lock for the whole stripe */
    Mutex m_lock;

    /** array with sample values */
    QArray<sample_t> m_samples;

    /** mutex for array of samples */
    Mutex m_lock_samples;

};

#endif /* _STRIPE_H_ */
