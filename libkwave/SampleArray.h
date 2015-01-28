/*************************************************************************
          SampleArray.h  -  array with Kwave's internal sample_t
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

#ifndef _SAMPLE_ARRAY_H_
#define _SAMPLE_ARRAY_H_

#include "config.h"

#include <QtCore/QSharedData>
#include <QtCore/QSharedDataPointer>

#include <kdemacros.h>

#include "libkwave/Sample.h"

namespace Kwave
{

    /**
     * array with sample_t, for use in KwaveSampleSource, KwaveSampleSink
     * and other streaming classes.
     */
    class KDE_EXPORT SampleArray
    {
    public:

	/** Default constructor, creates an empty array */
	SampleArray();

	/**
	 * Constructor, creates an array with predefined size
	 * (not initialized)
	 * @param size number of samples to hold
	 */
	explicit SampleArray(unsigned int size);

	/** Destructor */
	virtual ~SampleArray();

	/** returns a const pointer to the raw data (non-mutable) */
	inline const sample_t * constData() const
	{
	    if (KDE_ISUNLIKELY(!m_storage)) return 0;
	    if (m_storage->m_raw_data) return m_storage->m_raw_data;
	    return m_storage->m_data;
	}

	/** returns a pointer to the raw data (mutable) */
	inline sample_t *data() /* __attribute__((deprecated)) <- for debug */
	{
	    if (KDE_ISUNLIKELY(!m_storage)) return 0;
	    if (m_storage->m_raw_data) return m_storage->m_raw_data;
	    return m_storage->m_data;
	}

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

	/** fills the array with a sample value */
	void fill(sample_t value);

	/**
	 * operator [], non-const.
	 * @param index sample index [0...count()-1]
	 * @return reference to the requested sample (read/write)
	 */
	sample_t & operator [] (unsigned int index);

	/**
	 * operator [], non-const.
	 * @param index sample index [0...count()-1]
	 * @return reference to the requested sample (read only)
	 */
	const sample_t & operator [] (unsigned int index) const;

	/**
	 * Resizes the array. Using raw data mode is not allowed and will
	 * lead to an assert!
	 * @param size new number of samples
	 * @return true if succeeded, false if failed
	 */
	bool resize(unsigned int size) KDE_MUST_USE_RESULT;

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
	inline bool isEmpty() const { return (size() == 0); }

    private:

	class SampleStorage: public QSharedData {
	public:

	    /** default constructor */
	    SampleStorage();

	    /** copy constructor */
	    SampleStorage(const SampleStorage &other);

	    /** destructor */
	    virtual ~SampleStorage();

	    /**
	     * Resizes the array. Using raw data mode is not allowed and will
	     * lead to an assert!
	     * @param size new number of samples
	     */
	    void resize(unsigned int size);

	public:
	    /** size in samples */
	    unsigned int m_size;

	    /** pointer to the area with the samples (allocated) */
	    sample_t *m_data;

	    /** pointer to some raw data that has been set */
	    sample_t *m_raw_data;
	};

	QSharedDataPointer<SampleStorage> m_storage;
    };
}

#endif /* _SAMPLE_ARRAY_H_ */

//***************************************************************************
//***************************************************************************
