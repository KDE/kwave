/***************************************************************************
          TrackWriter.h  -  stream for writing samples into a track
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

#ifndef TRACK_WRITER_H
#define TRACK_WRITER_H

#include "config.h"

#include <QtGlobal>
#include <QElapsedTimer>
#include <QObject>

#include "libkwave/Writer.h"

namespace Kwave
{

    class SampleArray;
    class Track;

    /**
     * @class TrackWriter
     * Input stream for transferring samples into a Track.
     *
     * @warning THIS CLASS IS NOT THREADSAFE! It is intended to be owned by
     *          and used from only one thread.
     */
    class Q_DECL_EXPORT TrackWriter: public Kwave::Writer
    {
        Q_OBJECT
    public:
        /**
         * Constructor, creates a writer for write access to a track.
         *
         * @param track reference to the track
         * @param mode specifies where and how to insert
         * @param left start of the input (only useful in insert and
         *             overwrite mode)
         * @param right end of the input (only useful with overwrite mode)
         * @see InsertMode
         */
        TrackWriter(Kwave::Track &track, Kwave::InsertMode mode,
            sample_index_t left = 0, sample_index_t right = 0);

        /**
         * Destructor.
         */
        virtual ~TrackWriter() Q_DECL_OVERRIDE;

        /**
         * Flush the content of a buffer. Normally the buffer is the
         * internal intermediate buffer used for single-sample writes.
         * When using block transfers, the internal buffer is bypassed
         * and the written block is passed instead.
         * @internal
         * @param buffer reference to the buffer to be flushed
         * @param count number of samples in the buffer to be flushed,
         *              will be internally set to zero if successful
         * @return true if successful, false if failed (e.g. out of memory)
         */
        virtual bool write(const Kwave::SampleArray &buffer, unsigned int &count)
            Q_DECL_OVERRIDE;

    private:

        /** the track that receives our data */
        Kwave::Track &m_track;

        /** timer for limiting the number of progress signals per second */
        QElapsedTimer m_progress_time;

    };

}

#endif /* TRACK_WRITER_H */

//***************************************************************************
//***************************************************************************
