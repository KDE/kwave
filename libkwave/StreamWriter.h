/***************************************************************************
          StreamWriter.h - adapter between writers and sample source
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

#ifndef STREAM_WRITER_H
#define STREAM_WRITER_H

#include "config.h"

#include <QtGlobal>
#include <QObject>

#include "libkwave/Writer.h"

namespace Kwave
{

    class SampleArray;
    class Track;

    /**
     * @class StreamWriter
     * Input stream for transferring samples into a Track.
     *
     * @warning THIS CLASS IS NOT THREADSAFE! It is intended to be owned by
     *          and used from only one thread.
     */
    class Q_DECL_EXPORT StreamWriter: public Kwave::Writer
    {
	Q_OBJECT
    public:

	/**
	 * Constructor
	 */
	StreamWriter();

	/**
	 * Destructor.
	 */
	virtual ~StreamWriter();

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
	 bool write(const Kwave::SampleArray &buffer,
	                    unsigned int &count) Q_DECL_OVERRIDE;

    signals:

	/** emits a block with sine wave data */
	void output(Kwave::SampleArray data);

    };

}

#endif /* STREAM_WRITER_H */

//***************************************************************************
//***************************************************************************
