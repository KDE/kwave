/***************************************************************************
      MultiTrackWriter.h - writer for multi-track signals
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

#ifndef _MULTI_TRACK_WRITER_H_
#define _MULTI_TRACK_WRITER_H_

#include <qvector.h>

class MultiTrackReader;
class SampleWriter;

/**
 * A MultiTrackWriter encapsulates a set of <c>SampleWriter</c>s for
 * easier use of multi-track signals.
 */
class MultiTrackWriter: public QVector<SampleWriter>
{
public:

    virtual ~MultiTrackWriter() {};

    /**
     * Transfers the content of multiple tracks into the destination.
     * Works the same way as the corresponding operator in
     * <c>SampleWriter</c>, but for multiple tracks. If the number of
     * tracks in source and destination do not match, the tracks will
     * be mixed up / down.
     */
    MultiTrackWriter &operator << (const MultiTrackReader &source);

};

#endif /* _MULTI_TRACK_WRITER_H_ */
