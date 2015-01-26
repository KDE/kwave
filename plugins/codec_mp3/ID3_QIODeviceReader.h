/*************************************************************************
    ID3_QIODeviceReader.h  -  Adapter between QIODevice and ID3_Reader
                             -------------------
    begin                : Wed Aug 14 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#ifndef _ID3_QIODEVICE_READER_H_
#define _ID3_QIODEVICE_READER_H_

#include <config.h>

#include <id3/globals.h>
#include <id3/reader.h>

class QIODevice;

namespace Kwave
{
    /**
     * @class ID3_QIODeviceReader
     * Adapter between QIODevice and ID3_Reader
     */
    class ID3_QIODeviceReader: public ID3_Reader
    {
    public:

	/** Constructor */
	ID3_QIODeviceReader(QIODevice &source);

	/** Destructor */
	virtual ~ID3_QIODeviceReader();

	/** Close the source. Not implemented. */
	virtual void close();

	/** Get the start position, always zero */
	virtual ID3_Reader::pos_type getBeg();

	/** Get the end position, identical to size()-1 */
	virtual ID3_Reader::pos_type getEnd();

	/** Returns the current position */
	virtual ID3_Reader::pos_type getCur();

	/** Sets a new position and returns the new one */
	virtual ID3_Reader::pos_type setCur(ID3_Reader::pos_type pos=0);

	/** Reads out one single character */
	virtual ID3_Reader::int_type readChar();

	/** Reads one character without advancing the current position */
	virtual ID3_Reader::int_type peekChar();

	/** Read out a block of characters */
	virtual size_type readChars(char_type buf[], size_type len);
	virtual size_type readChars(char buf[], size_type len);

    private:

	/** reference to a QIODevice that is used as source */
	QIODevice &m_source;
    };
}

#endif /* _ID3_QIODEVICE_READER_H_ */

//***************************************************************************
//***************************************************************************
