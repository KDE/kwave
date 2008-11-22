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

#include "config.h"

#include <QObject>
#include <QList>

#include <kdemacros.h>

#include "libkwave/SampleReader.h"
#include "libkwave/KwaveMultiTrackSource.h"

class SignalManager;

/**
 * A MultiTrackReader encapsulates a set of <c>SampleReader</c>s for
 * easier use of multi-track signals.
 */
class KDE_EXPORT MultiTrackReader
    :public Kwave::MultiTrackSource<SampleReader, false>
{
    Q_OBJECT
private:

    /** Default constructor */
    MultiTrackReader();

public:

    /**
     * Constructor
     * @param signal_manager reference to a SignalManager
     * @param track_list array of indices of tracks for writing
     * @param left index of the first sample
     * @param right index of the last sample
     */
    MultiTrackReader(SignalManager &signal_manager,
                     const QList<unsigned int> &track_list,
                     unsigned int first, unsigned int last);

    /** Destructor */
    virtual ~MultiTrackReader();

    /**
     * Returns the offset of the reader, as
     * passed to the constructor as "first"
     */
    virtual unsigned int first() const;

    /**
     * Returns the last sample offset of the reader, as
     * passed to the constructor as "last"
     */
    virtual unsigned int last() const;

    /** Returns true if one of the readers has reached eof() */
    virtual bool eof() const;

    /** @see QList::isEmpty() */
    inline virtual bool isEmpty() const {
        return (Kwave::MultiTrackSource<SampleReader, false>::tracks() < 1);
    };

    /** returns true if the transfer has been canceled */
    inline bool isCanceled() const { return m_canceled; };

    /** @see QList::insert() */
    virtual bool insert(unsigned int track, SampleReader *reader);

signals:

    /**
     * Emits the current progress in totally processed samples, range is
     * from zero to the (length of the writer * number of tracks) - 1.
     */
    void progress(unsigned int samples);

public slots:

    /**
     * Can be connected to some progress dialog to cancel the current
     * transfer.
     */
    void cancel();

    /** Resets all readers to zero */
    void reset();

private slots:

    /**
     * Connected to each SampleReader to get informed about their progress.
     */
    void proceeded();

protected:

    /** index of the first sample to read */
    unsigned int m_first;

    /** index of the last sample to read */
    unsigned int m_last;

    /**
     * Initialized as false, will be true if the transfer has
     * been canceled
     */
    bool m_canceled;

};

#endif /* _MULTI_TRACK_READER_H_ */
