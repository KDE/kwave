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

#include <qobject.h>
#include <qarray.h>

#include "mt/Mutex.h"

#include "libkwave/Sample.h"

//***************************************************************************
class Stripe: public QObject
{
    Q_OBJECT
public:

    /**
     * Default constructor. Creates an empty stripe with zero-length.
     */
    Stripe();

    /**
     * Constructor. Creates a new zero-length stripe.
     * @param start position within the track
     */
    Stripe(unsigned int start);

    /**
     * Constructor. Creates a stripe that already contains samples.
     * @param start position within the track
     * @param samples array with initial samples
     */
    Stripe(unsigned int start, const QArray<sample_t> &samples);

    /**
     * Destructor.
     */
    virtual ~Stripe();

    /**
     * Returns the start position of the stripe within the track.
     */
    unsigned int start();

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
     * Appends an array of samples to the end of the stripe.
     * @param samples array with the samples
     * @param count number of samples in the array
     * @return number of samples appended
     */
    unsigned int append(const QArray<sample_t> &samples, unsigned int count);

    /**
     * Inserts an array of samples into the stripe.
     * @param samples array with the samples
     * @param offset position within the stripe, relative to the start of
     *        the stripe [0...length()-1]
     * @param count number of samples in the array
     * @return number of samples appended
     */
    unsigned int insert(const QArray<sample_t> &samples, unsigned int offset,
                        unsigned int count);

    /**
     * Deletes a range of samples
     * @param offset index of the first sample
     * @param length number of samples
     */
    void deleteRange(unsigned int offset, unsigned int length);

    /**
     * Copies the content of an array of samples into the stripe.
     * @param offset the offset within the stripe (target)
     * @param samples array of samples to be copied
     * @param srcoff offset within the source array
     * @param srclen length of the data in the source array
     * @warning this method is intended to be used only internally
     *          and waives any error-checking in order to be fast!
     */
    void overwrite(unsigned int offset, const QArray<sample_t> &samples,
    	unsigned int srcoff, unsigned int srclen);


    /**
     * Reads out samples from the stripe into a buffer
     * @param buffer array for samples to be read (destination)
     * @param dstoff offset within the destination buffer
     * @param offset the offset within the stripe (source)
     * @param length number of samples to read
     * @return number of samples read
     * @warning this method is intended to be used only internally
     *          and waives any error-checking in order to be fast!
     */
    unsigned int read(QArray<sample_t> &buffer, unsigned int dstoff,
	unsigned int offset, unsigned int length);

    /**
     * Operator for appending an array of samples to the
     * end of the stripe.
     */
    Stripe &operator << (const QArray<sample_t> &samples);

signals:

    /**
     * Emitted if the stripe has grown. This implies a modification of
     * the inserted data, so no extra sigSamplesModified is emitted.
     * @param src source stripe of the signal (*this)
     * @param offset position from which the data was inserted
     * @param length number of samples inserted
     * @see sigSamplesModified
     */
    void sigSamplesInserted(Stripe &src, unsigned int offset,
                            unsigned int length);

    /**
     * Emitted if data has been removed from the stripe.
     * @param src source stripe of the signal (*this)
     * @param offset position from which the data was removed
     * @param length number of samples deleted
     */
    void sigSamplesDeleted(Stripe &src, unsigned int offset,
                           unsigned int length);

    /**
     * Emitted if some data within the stripe has been modified.
     * @param src source stripe of the signal (*this)
     * @param offset position from which the data was modified
     * @param length number of samples modified
     */
    void sigSamplesModified(Stripe &src, unsigned int offset,
                            unsigned int length);

private:

    /** start position within the track */
    unsigned int m_start;

    /** array with sample values */
    QArray<sample_t> m_samples;

    /** mutex for array of samples */
    Mutex m_lock_samples;

};

#endif /* _STRIPE_H_ */
