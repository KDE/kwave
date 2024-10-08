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

#ifndef MULTI_TRACK_READER_H
#define MULTI_TRACK_READER_H

#include "config.h"
#include "libkwave_export.h"

#include <QtGlobal>
#include <QList>
#include <QObject>
#include <QVector>

#include "libkwave/MultiTrackSource.h"
#include "libkwave/SampleReader.h"

namespace Kwave
{

    class SignalManager;

    /**
     * A MultiTrackReader encapsulates a set of <c>SampleReader</c>s for
     * easier use of multi-track signals.
     */
    class LIBKWAVE_EXPORT MultiTrackReader
        :public Kwave::MultiTrackSource<Kwave::SampleReader, false>
    {
        Q_OBJECT
    private:

        /** Default constructor */
        MultiTrackReader();

    public:

        /**
         * Constructor
         * @param mode a reader mode, see Kwave::ReaderMode
         * @param signal_manager reference to a SignalManager
         * @param track_list array of indices of tracks for writing
         * @param first index of the first sample (left)
         * @param last index of the last sample (right)
         */
        MultiTrackReader(Kwave::ReaderMode mode,
                        Kwave::SignalManager &signal_manager,
                        const QVector<unsigned int> &track_list,
                        sample_index_t first, sample_index_t last);

        /** Destructor */
        ~MultiTrackReader() override;

        /**
         * Returns the offset of the reader, as
         * passed to the constructor as "first"
         */
        virtual sample_index_t first() const;

        /**
         * Returns the last sample offset of the reader, as
         * passed to the constructor as "last"
         */
        virtual sample_index_t last() const;

        /** Returns true if one of the readers has reached eof() */
        virtual bool eof() const;

        /** @see QList::isEmpty() */
        inline virtual bool isEmpty() const {
            return (Kwave::MultiTrackSource<Kwave::SampleReader,
                    false>::tracks() < 1);
        }

        /** @see QList::insert() */
        virtual bool insert(unsigned int track, Kwave::SampleReader *reader)
            override;

        /** Skips a number of samples. */
        virtual void skip(sample_index_t count);

        /** Seeks to a given position */
        virtual void seek(sample_index_t pos);

    signals:

        /**
         * Emits the current progress in percent [0...100].
         */
        void progress(qreal percent);

    public slots:

        /** Resets all readers to zero */
        void reset();

    private slots:

        /**
         * Connected to each SampleReader to get informed about their progress.
         */
        void proceeded();

    protected:

        /** index of the first sample to read */
        sample_index_t m_first;

        /** index of the last sample to read */
        sample_index_t m_last;

    };
}

#endif /* MULTI_TRACK_READER_H */

//***************************************************************************
//***************************************************************************
