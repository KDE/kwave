/***************************************************************************
            ClipBoard.h  -  the Kwave clipboard
			     -------------------
    begin                : Tue Jun 26, 2001
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

#ifndef _CLIP_BOARD_H_
#define _CLIP_BOARD_H_

#include "config.h"
#include <qmemarray.h>
#include <qptrlist.h>
#include "mt/SharedLock.h"

class MultiTrackReader;
class Signal;
class Track;

/**
 * Implements a global clipboard for Kwave. It supports only the three
 * simple operations <c>put</c>, <c>get</c> and <c>clear</c>.
 *
 */
class ClipBoard
{
public:

    /** Constructor. */
    ClipBoard();

    /** Destructor */
    virtual ~ClipBoard();

    /**
     * Discards the current content of the clipboard and fills
     * it with a selected range of samples from a set of tracks.
     * @param signal the Signal with the tracks to read from
     * @param track_list a list of indices of tracks
     * @param offset first sample to copy
     * @param length number of samples
     * @param rate sample rate [samples/second]
     * @todo support for multiple stripes
     */
    void copy(Signal &signal, const QMemArray<unsigned int> &track_list,
              unsigned int offset, unsigned int length, double rate);

    /**
     * Returns a MultiTrackReader for reading the clipboard content.
     * @param readers reference to the MultiTrackReader to be filled.
     * @note the returned vector has set "autoDelete" to true, so you
     *       don't have to care about cleaning up
     * @see SampleReader
     * @see selectedTracks()
     */
    void openMultiTrackReader(MultiTrackReader &readers);

    /**
     * Clears the internal buffers. The clipboard will be empty after this
     * function returns and the sample rate will be set to zero.
     */
    void clear();

    /**
     * Returns the length of the clipboard content [samples]
     */
    unsigned int length();

    /**
     * Returns the sample rate of the buffer content [samples/second].
     */
    double rate();

    /**
     * Returns true if the buffer is empty.
     */
    bool isEmpty();

    /**
     * Returns the number of tracks in the buffer or zero if the
     * buffer is empty.
     */
    unsigned int tracks();

private:

    /** Lock for exclusive or readonly access. */
    SharedLock m_lock;

    /** Sample rate of the buffer content. */
    double m_rate;

    /** Internal buffer, implemented as a list of tracks */
    QPtrList<Track> m_buffer;

};

#endif /* _CLIP_BOARD_H_ */
