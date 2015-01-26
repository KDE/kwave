/*************************************************************************
    OggSubDecoder.h  -  sub decoder base class for audio in an Ogg container
                             -------------------
    begin                : Wed Dec 26 2012
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

#ifndef _OGG_SUB_DECODER_H_
#define _OGG_SUB_DECODER_H_

#include <config.h>

class QWidget;

namespace Kwave
{
    class FileInfo;
    class MultiWriter;

    class OggSubDecoder
    {
    public:
	/** Destructor */
	virtual ~OggSubDecoder() {}

	/**
	 * parse the header of the stream and initialize the decoder
	 * @param widget a QWidget to be used as parent for error messages
	 * @param info reference to a FileInfo to fill
	 * @return -1 if failed or +1 if succeeded
	 */
	virtual int open(QWidget *widget, Kwave::FileInfo &info) = 0;

	/**
	 * decode received ogg data
	 * @param dst a MultiWriter to be used as sink
	 * @return -1 if failed or >= 0 if succeeded
	 */
	virtual int decode(Kwave::MultiWriter &dst) = 0;

	/** reset the stream info */
	virtual void reset() = 0;

	/**
	 * finish the decoding, last chance to fix up some file info
	 * @param info reference to a FileInfo to fill
	 */
	virtual void close(Kwave::FileInfo &info) = 0;

    };
}

#endif /* _OGG_SUB_DECODER_H_ */

//***************************************************************************
//***************************************************************************
