/*************************************************************************
  ID3_QIODeviceWriter.h  -  Adapter between QIODevice and ID3_Writer
                             -------------------
    begin                : Mon May 28 2012
    copyright            : (C) 2012 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ID3_QIODEVICE_WRITER_H
#define ID3_QIODEVICE_WRITER_H

#include "config.h"

#include <id3/globals.h>
#include <id3/writer.h>

class QIODevice;

namespace Kwave
{
    /**
     * @class ID3_QIODeviceWriter
     * Adapter between QIODevice and ID3_Writer
     */
    class ID3_QIODeviceWriter: public ID3_Writer
    {
    public:

	/** Constructor */
	explicit ID3_QIODeviceWriter(QIODevice &dest);

	/** Destructor */
	virtual ~ID3_QIODeviceWriter();

	/** Close the destination. Not implemented. */
	virtual void close() Q_DECL_OVERRIDE;

	/** Flush the destination, not implemented (not needed) */
	virtual void flush() Q_DECL_OVERRIDE;

	/** Get the start position, always zero */
	virtual ID3_Writer::pos_type getBeg() Q_DECL_OVERRIDE;

	/** Get the end position, identical to size()-1 */
	virtual ID3_Writer::pos_type getEnd() Q_DECL_OVERRIDE;

	/** Returns the current position */
	virtual ID3_Writer::pos_type getCur() Q_DECL_OVERRIDE;

	/** Returns the number of bytes written */
	virtual ID3_Writer::size_type getSize() Q_DECL_OVERRIDE;

	/** Returns the maximum number of bytes written */
	virtual ID3_Writer::size_type getMaxSize() Q_DECL_OVERRIDE;

	/**
	 * Write up to len characters into buf and advance the internal position
	 * accordingly.
	 */
	virtual ID3_Writer::size_type writeChars(
	    const ID3_Writer::char_type buf[], ID3_Writer::size_type len)
	    Q_DECL_OVERRIDE;

	virtual ID3_Writer::size_type writeChars(
	    const char buf[], ID3_Writer::size_type len)
	    Q_DECL_OVERRIDE;

	/** returns true if the writer has readed eof (always false) */
	virtual bool atEnd() Q_DECL_OVERRIDE;

    private:

	/** reference to a QIODevice that is used as destination */
	QIODevice &m_dest;

	/** number of bytes written */
	ID3_Writer::size_type m_written;
    };
}

#endif /* ID3_QIODEVICE_WRITER_H */

//***************************************************************************
//***************************************************************************
