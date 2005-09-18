/***************************************************************************
        SampleFormat.h  -  Map for all known sample formats
                             -------------------
    begin                : Sun Jul 28 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#ifndef _SAMPLE_FORMAT_H_
#define _SAMPLE_FORMAT_H_

#include "config.h"
#include <audiofile.h>
#include "TypesMap.h"

class SampleFormat
{
public:
    typedef enum {
	Unknown  = -1,                   /**< unknown/invalid format */
	Signed   = AF_SAMPFMT_TWOSCOMP,  /**< signed integer */
	Unsigned = AF_SAMPFMT_UNSIGNED,  /**< unsigned integer */
	Float    = AF_SAMPFMT_FLOAT,     /**< 32 bit floating point */
	Double   = AF_SAMPFMT_DOUBLE     /**< 64 bit floating point */
    } Format;

    /** Default constructor */
    SampleFormat() { assign(Unknown); };

    /** Constructor, from SampleFormat::xxx */
    SampleFormat(const Format x) { assign(x); };

    /** Copy constructor */
    SampleFormat(const SampleFormat &f) { assign(f); };

    /** Destructor */
    virtual ~SampleFormat() {};

    /** conversion operator to sample_format_t */
    inline operator Format() const { return m_format; };

    /** assignment operator from sample_format_t */
    inline void assign(Format f) { m_format = f; };

    /** compare operator */
    inline bool operator == (const Format &f) const {
	return (f == m_format);
    };

    /** conversion to int */
    inline int toInt() const { return static_cast<int>(m_format); };

    /** conversion from int */
    void fromInt(int i);

private:

    /** internal storage of the sample format, see sample_format_t */
    Format m_format;

public:

    /** map for translating between index, sample format and name */
    class Map: public TypesMap<int, Format>
    {
    public:
	/** Constructor */
	Map();

	/** Destructor */
	virtual ~Map();

	/** fills the list */
	virtual void fill();
    };

};

#endif /* _SAMPLE_FORMAT_H_ */
