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
#include <qobject.h>
#include <qvector.h>

#include "libkwave/SampleReader.h"

/**
 * A MultiTrackReader encapsulates a set of <c>SampleReader</c>s for
 * easier use of multi-track signals.
 */
class MultiTrackReader: public QObject, private QVector<SampleReader>
{
    Q_OBJECT
public:

    /** Constructor */
    MultiTrackReader();

    /** Destructor */
    virtual ~MultiTrackReader() {};

    /** Returns true if one of the readers has reached eof() */
    virtual bool eof() const;

    /** @see QVector::operator[] */
    inline virtual SampleReader* operator[] (int i) const {
	return QVector<SampleReader>::at(i);
    };

    /** @see QVector::count() */
    inline virtual unsigned int count() const {
	return QVector<SampleReader>::count();
    };

    /** @see QVector::clear() */
    inline virtual void clear() { QVector<SampleReader>::clear(); };

    /** @see QVector::isEmpty() */
    inline virtual bool isEmpty() {
        return QVector<SampleReader>::isEmpty();
    };

    /** @see QVector::insert() */
    virtual bool insert(unsigned int track, const SampleReader *reader);

    /** @see QVector::resize() */
    virtual bool resize(unsigned int size) {
        return QVector<SampleReader>::resize(size);
    };

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

    /** Resets all readers to zero */
    void reset();
    
private slots:

    /**
     * Connected to each SampleReader to get informed about their progress.
     */
    void proceeded();

protected:

    /**
     * Initialized as false, will be true if the transfer has
     * been cancelled
     */
    bool m_cancelled;

};

#endif /* _MULTI_TRACK_READER_H_ */
