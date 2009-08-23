/***************************************************************************
           MultiWriter.h - writer for multi-track processing
			     -------------------
    begin                : Sun Aug 23 2009
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

#ifndef _MULTI_WRITER_H_
#define _MULTI_WRITER_H_

#include "config.h"

#include <QObject>
#include <QList>

#include <kdemacros.h>

#include "libkwave/Writer.h"
#include "libkwave/KwaveMultiTrackSink.h"

class MultiTrackReader;

namespace Kwave {

    /**
     * A MultiWriter encapsulates a set of <c>Writer</c>s for
     * easier use of multi-track signals.
     */
    class KDE_EXPORT MultiWriter: public Kwave::MultiTrackSink<Kwave::Writer>
    {
	Q_OBJECT
    public:

	/** Default constructor */
	MultiWriter();

	/** Destructor */
	virtual ~MultiWriter();

	/** Returns the last sample index of all streams */
	virtual unsigned int last() const;

	/** Flushes all streams */
	virtual void flush();

	/** @see Kwave::MultiTrackSink<Kwave::Writer>::clear() */
	virtual void clear();

	/** @see Kwave::MultiTrackSink<Kwave::Writer>::insert() */
	virtual bool insert(unsigned int track, Kwave::Writer *writer);

	/** returns true if the transfer has been canceled */
	inline bool isCanceled() const { return m_canceled; }

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
	 * Connected to each Writer to get informed about their progress.
	 */
	void proceeded();

    protected:

	/**
	 * Initialized as false, will be true if the transfer has
	 * been canceled
	 */
	bool m_canceled;

    };

}

#endif /* _MULTI_WRITER_H_ */
