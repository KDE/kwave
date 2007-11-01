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

#include "config.h"
#include <qobject.h>
#include <qptrvector.h>
#include "libkwave/SampleWriter.h"
#include "libkwave/KwaveMultiTrackSink.h"

class MultiTrackReader;
class SignalManager;

/**
 * A MultiTrackWriter encapsulates a set of <c>SampleWriter</c>s for
 * easier use of multi-track signals.
 */
class MultiTrackWriter:
    public Kwave::MultiTrackSink<SampleWriter>
{
    Q_OBJECT

private:
    /** Default constructor */
    MultiTrackWriter();

public:
    /**
     * Constructor
     * @param signal_manager reference to a SignalManager
     * @param track_list array of indices of tracks for reading
     * @param mode specifies where and how to insert
     * @param left index of the first sample
     * @param right index of the last sample
     */
    MultiTrackWriter(SignalManager &signal_manager,
                     const QMemArray<unsigned int> &track_list,
                     InsertMode mode,
                     unsigned int left, unsigned int right);

    /**
     * Constructor that opens a set of SampleWriters using the currently
     * selected list of tracks and the current selection. If nothing is
     * selected, the whole signal will be selected.
     *
     * @param signal_manager reference to a SignalManager
     * @param writers reference to a vector that receives all writers.
     * @param mode specifies where and how to insert
     */
    MultiTrackWriter(SignalManager &signal_manager, InsertMode mode);

    /** Destructor */
    virtual ~MultiTrackWriter();

    /**
     * Transfers the content of multiple tracks into the destination.
     * Works the same way as the corresponding operator in
     * <c>SampleWriter</c>, but for multiple tracks. If the number of
     * tracks in source and destination do not match, the tracks will
     * be mixed up / down.
     */
    MultiTrackWriter &operator << (const MultiTrackReader &source);

    /** Returns the last sample index of all streams */
    unsigned int last();

    /** Flushes all streams */
    void flush();

    /** @see QPtrVector::clear() */
    virtual void clear();

    /** @see QPtrVector::insert() */
    virtual bool insert(unsigned int track, SampleWriter *writer);

    /** returns true if the transfer has been cancelled */
    inline bool isCancelled() { return m_cancelled; };

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

private slots:

    /**
     * Connected to each SampleWriter to get informed about their progress.
     */
    void proceeded();

protected:

    /**
     * Initialized as false, will be true if the transfer has
     * been cancelled
     */
    bool m_cancelled;

};

#endif /* _MULTI_TRACK_WRITER_H_ */
