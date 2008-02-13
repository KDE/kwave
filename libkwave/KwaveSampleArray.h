/*************************************************************************
    KwaveSampleArray.h  -  array with Kwave's internal sample_t
                             -------------------
    begin                : Sun Oct 07 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KWAVE_SAMPLE_ARRAY_H_
#define _KWAVE_SAMPLE_ARRAY_H_

#include "config.h"

#include <QSharedData>
#include <QSharedDataPointer>

#include "libkwave/Sample.h"

namespace Kwave {
    /**
     * array with sample_t, for use in KwaveSampleSource, KwaveSampleSink
     * and other streaming classes.
     */
    class SampleArray
    {
    public:

	/** Default constructor, creates an empty array */
	SampleArray();

	/**
	 * Constructor, creates an array with predefined size
	 * (not initialized)
	 * @param size number of samples to hold
	 */
	SampleArray(unsigned int size);

	/** Destructor */
	virtual ~SampleArray();

	/** returns a const pointer to the raw data (non-mutable) */
	sample_t * const data() const;

	/** returns a pointer to the raw data (mutable) */
	sample_t *data();

	/**
	 * Sets a pointer to some raw data
	 * @param data pointer to an array of sample_t
	 * @param size number of samples in the array
	 */
	void setRawData(sample_t *data, unsigned int size);

	/**
	 * Resets the raw data set with setRawData() and resizes this
	 * array to zero-size
	 */
	void resetRawData();

	/**
	 * operator [], non-const.
	 * @param index sample index [0...count()-1]
	 * @return reference to the requested sample (read only)
	 */
	sample_t & operator [] (unsigned int index);

	/**
	 * operator [], non-const.
	 * @param index sample index [0...count()-1]
	 * @return reference to the requested sample (read/write)
	 */
	const sample_t & operator [] (unsigned int index) const;

	/**
	 * Resizes the array. If it has been using raw data, the raw
	 * data is reset through resetRawData().
	 * @param size new number of samples
	 */
	void resize(unsigned int size);

	/**
	 * Returns the number of samples.
	 * @return samples [0...N]
	 */
	unsigned int size() const;

	/**
	 * Returns whether the array is empty.
	 * The same as (size() == 0).
	 * @return true if empty, false if not
	 */
	inline bool isEmpty() const { return (size() == 0); };

    private:

	class SampleStorage: public QSharedData {
	public:

	    SampleStorage();

	    SampleStorage(const SampleStorage &other);

	    virtual ~SampleStorage();

	    unsigned int m_size;

	    sample_t *m_data;
	};

	QSharedDataPointer<SampleStorage> m_storage;
    };
}

#endif /* _KWAVE_SAMPLE_ARRAY_H_ */
