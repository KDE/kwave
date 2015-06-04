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

#ifndef MULTI_WRITER_H
#define MULTI_WRITER_H

#include "config.h"

#include <QtGlobal>
#include <QList>
#include <QObject>

#include "libkwave/MultiTrackSink.h"
#include "libkwave/Writer.h"

namespace Kwave
{

    /**
     * A MultiWriter encapsulates a set of <c>Writer</c>s for
     * easier use of multi-track signals.
     */
    class Q_DECL_EXPORT MultiWriter
	:public Kwave::MultiTrackSink<Kwave::Writer, false>
    {
	Q_OBJECT
    public:

	/** Default constructor */
	MultiWriter();

	/** Destructor */
	virtual ~MultiWriter();

	/** Returns the last sample index of all streams */
	virtual sample_index_t last() const;

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
	 * Emits the current progress in percent, if the writers
	 * are in "overwrite" mode
	 * @see written for insert and append mode
	 */
	void progress(qreal percent);

	/**
	 * Emitts the currently written samples, summed up over all
	 * tracks, if the writers are in "insert" or "append" mode
	 * @see progress for overwrite mode
	 */
	void written(quint64 samples);

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

#endif /* MULTI_WRITER_H */

//***************************************************************************
//***************************************************************************
