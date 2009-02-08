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

#include "config.h"

#include <QObject>
#include <QMutex>

#include <kdemacros.h>

#include "libkwave/KwaveSampleArray.h"
#include "libkwave/Sample.h"

//***************************************************************************
class KDE_EXPORT Stripe: public QObject
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
     * Constructor. Creates a stripe that already contains samples,
     * copied from a buffer with samples.
     *
     * @param start position within the track
     * @param samples array with initial samples
     */
    Stripe(unsigned int start, const Kwave::SampleArray &samples);

    /**
     * Constructor. Creates a stripe that already contains samples,
     * copied from another stripe with offset.
     *
     * @param start position within the track
     * @param stripe source stripe to copy from
     * @param offset offset within the source stripe
     */
    Stripe(unsigned int start, Stripe &stripe, unsigned int offset);

    /**
     * Destructor.
     */
    virtual ~Stripe();

    /**
     * Returns the start position of the stripe within the track.
     */
    unsigned int start();

    /**
     * Sets a new start position for the stripe
     */
    void setStart(unsigned int start);

    /**
     * Returns the current length of the stripe in samples.
     */
    unsigned int length();

    /**
     * Returns the position of the last sample of the stripe,
     * same as (start() + length() ? (length() - 1))
     */
    unsigned int end();

    /**
     * Resizes the stripe to a new number of samples. If the array
     * size is reduced, samples from the end are thrown away. If
     * the size is increased, samples with zero value will be added
     * to the end.
     * @param length new length of the array [samples]
     * @param initialize if true, initialize all new areas with
     *                   zeroes if the size has been increased
     *                   (default = true)
     * @return new length [samples]
     */
    unsigned int resize(unsigned int length, bool initialize = true);

    /**
     * Appends an array of samples to the end of the stripe.
     * @param samples array with the samples
     * @param offset the offset within the array
     * @param count number of samples in the array
     * @return number of samples appended
     */
    unsigned int append(const Kwave::SampleArray &samples,
                        unsigned int offset,
                        unsigned int count);

    /**
     * Deletes a range of samples
     * @param offset index of the first sample, relative to the start of
     *        the stripe [0...length()-1]
     * @param length number of samples
     */
    void deleteRange(unsigned int offset, unsigned int length);

    /**
     * Copies the content of an array of samples into the stripe.
     * @param offset the offset within the stripe (target)
     * @param source array of samples to be copied
     * @param srcoff offset within the source array
     * @param srclen length of the data in the source array
     * @warning this method is intended to be used only internally
     *          and lacks any error-checking in order to be fast!
     */
    void overwrite(unsigned int offset, const Kwave::SampleArray &source,
    	unsigned int srcoff, unsigned int srclen);


    /**
     * Reads out samples from the stripe into a buffer
     *
     * @param buffer array for samples to be read (destination)
     * @param dstoff offset within the destination buffer
     * @param offset the offset within the stripe (source)
     * @param length number of samples to read
     * @return number of samples read
     * @warning this method is intended to be used only internally
     *          and lacks any error-checking in order to be fast!
     */
    unsigned int read(Kwave::SampleArray &buffer, unsigned int dstoff,
	unsigned int offset, unsigned int length);

    /**
     * Returns the minumum and maximum sample value within a range
     * of samples.
     * @param first index of the first sample
     * @param last index of the last sample
     * @param min receives the lowest value or 0 if no samples are in range
     * @param max receives the highest value or 0 if no samples are in range
     */
    void minMax(unsigned int first, unsigned int last,
                sample_t &min, sample_t &max);

    /**
     * Operator for appending an array of samples to the
     * end of the stripe.
     */
    Stripe &operator << (const Kwave::SampleArray &samples);

protected:

    /**
     * Resizes the internal storage.
     * @param length the new length in samples
     * @return the length after the resize operation. Should be equal
     *         to the length that has been given as parameter. If not,
     *         something has failed.
     */
    unsigned int resizeStorage(unsigned int length);

private:

    /**
     * Guard for mapping the storage into memory
     */
    class MapStorageGuard
    {
    public:
	/**
	 * Constructor
	 * @param stripe should be *this of the stripe
	 */
	MapStorageGuard(Stripe &stripe);

	/** Destructor */
	virtual ~MapStorageGuard();

	/**
	 * Returns a pointer to the mapped memory, or null if
	 * the mapping has failed
	 */
	sample_t *storage();

    private:

        /** stripe which gets it's storage mapped */
        Stripe &m_stripe;

	/** pointer to the memory used for storage */
	sample_t *m_storage;

    };

    /**
     * Wrapper for mapping the storage into memory and accessing
     * it like a normal QMemArray<sample_t>. Should be used like
     * a guard, internally uses a MapstorageGuard.
     */
    class MappedArray: public Kwave::SampleArray
    {
    public:
	/**
	 * Constructor
	 * @param stripe should be *this of the stripe
	 * @param length the length of the stripe [samples]
	 */
	MappedArray(Stripe &stripe, unsigned int length);

	/** Destructor */
	virtual ~MappedArray();

	/**
	 * Copy a portion of samples to another location,
	 * within the same storage.
	 *
	 * @param dst destination index [samples]
	 * @param src source index [samples]
	 * @param cnt number of samples
	 * @return cnt if succeeded or zero if the mapping has failed
	 * @note this is optimized for speed, no range checks!
	 */
	unsigned int copy(unsigned int dst, unsigned int src,
	                  unsigned int cnt);

	/**
	 * Copy a portion of samples from an array of samples.
	 *
	 * @param dst destination index [samples]
	 * @param source array with samples to copy from
	 * @param offset offset within the source array to start copy
	 * @param cnt number of samples
	 * @return cnt if succeeded or zero if the mapping has failed
	 * @note this is optimized for speed, no range checks!
	 */
	unsigned int copy(unsigned int dst,
	                  const Kwave::SampleArray &source,
	                  unsigned int offset, unsigned int cnt);

	/**
	 * Read a portion of samples into an array of samples.
	 *
	 * @param buffer array for samples to be read (destination)
	 * @param dstoff offset within the destination buffer
	 * @param offset the offset within the stripe (source)
	 * @param length number of samples to read
	 * @return length if succeeded or zero if the mapping has failed
	 * @warning this method is intended to be used only internally
	 *          and lacks any error-checking in order to be fast!
	 */
	unsigned int read(Kwave::SampleArray &buffer, unsigned int dstoff,
	                  unsigned int offset, unsigned int length);

    private:

	/** guard for mapping the storage */
	MapStorageGuard m_guard;

	/** length in samples */
	unsigned int m_length;
    };

private:

    /** maps the storage into memory */
    sample_t *mapStorage();

    /** unmaps the storage from memory */
    void unmapStorage();

private:

    /** start position within the track */
    unsigned int m_start;

    /** number of samples */
    unsigned int m_length;

    /** pointer/handle to a storage object */
    void *m_storage;

    /** mutex for array of samples */
    QMutex m_lock_samples;

    /** usage count of mapped storage */
    int m_map_count;

    /** mapped storage */
    sample_t *m_mapped_storage;

};

#endif /* _STRIPE_H_ */
