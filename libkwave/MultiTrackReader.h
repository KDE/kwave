/***************************************************************************
      MultiTrackReader.h - reader for multi-track signals
			     -------------------
    begin                : Sat Jun 30 2001
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

#ifndef _MULTI_TRACK_READER_H_
#define _MULTI_TRACK_READER_H_

#include <qvector.h>

class SampleReader;

/**
 * A MultiTrackReader encapsulates a set of <c>SampleReader</c>s for
 * easier use of multi-track signals.
 */
class MultiTrackReader: public QVector<SampleReader>
{
public:

    virtual ~MultiTrackReader() {};

};

#endif /* _MULTI_TRACK_READER_H_ */
