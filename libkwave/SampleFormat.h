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

class SampleFormat: public TypesMap<int, int>
{
public:
    enum {
	LinearTwosComplement = AF_SAMPFMT_TWOSCOMP,
	UnsignedInteger = AF_SAMPFMT_UNSIGNED,
	Float = AF_SAMPFMT_FLOAT,
	Double = AF_SAMPFMT_DOUBLE
    };

    /** Constructor */
    SampleFormat();

    /** Destructor */
    virtual ~SampleFormat();

    /** fills the list */
    virtual void fill();

};

#endif /* _SAMPLE_FORMAT_H_ */
