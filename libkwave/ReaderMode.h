/***************************************************************************
           ReaderMode.h  -  modes for reading samples from a track
                             -------------------
    begin                : May 31 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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

#ifndef READER_MODE_H
#define READER_MODE_H

namespace Kwave
{

    /**
     * operation modes for Kwave's SampleReader
     */
    typedef enum {
        SinglePassForward, /**<
                            * seekable in forward direction only, can be
                            * used only once. Passed stripes will be removed
                            * from the internal list and cannot be reached
                            * again.
                            * @note prefer this one wherever possible!
                            */
        SinglePassReverse, /**<
                            * seekable in backward direction only, like
                            * SinglePassForward but delivers samples in
                            * reverse order.
                            * @note useful for "reverse" effect or similar
                            */
        FullSnapshot       /**<
                            * seekable with random access, can be used
                            * several times. Passed stripes will never
                            * be discarded.
                            * @note use with care: can be very memory
                            *       expensive as it can produce a implicit
                            *       copy of the whole track!
                            */
    } ReaderMode;

}

#endif /* READER_MODE_H */

//***************************************************************************
//***************************************************************************
