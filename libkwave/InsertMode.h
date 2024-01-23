/***************************************************************************
           InsertMode.h  -  modes for inserting samples into a track
                             -------------------
    begin                : Feb 11 2001
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

#ifndef INSERT_MODE_H
#define INSERT_MODE_H

namespace Kwave
{
    /**
     * Modes for inserting data into a track.
     */
    typedef enum {
        Append,     /**<
                     * Append a new stripe to the end of the track. If
                     * the last stripe is zero-length, it will
                     * be used instead of a new one in order to avoid
                     * a trailing zero-length stripe.
                     */
        Insert,     /**<
                     * Insert data into a new stripe. If the position
                     * is within an existing stripe, that stripe will be
                     * split into two ones and the new stripe is inserted
                     * between them. If the position is after the end of
                     * the track, an empty stripe will be appended first
                     * to pad up to the wanted position.
                     */
        Overwrite   /**<
                     * Overwrite data within a given range. The range
                     * can span over several stripes. If the position is
                     * after the last sample, an empty stripe will be
                     * appended first to pad up to the wanted
                     * position. If the end of the track is reached, a
                     * new stripe will be appended.
                     */
    } InsertMode;
}

#endif /* INSERT_MODE_H */

//***************************************************************************
//***************************************************************************
