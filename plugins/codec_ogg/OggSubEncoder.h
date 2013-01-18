/*************************************************************************
    OggSubEncoder.h  -  sub encoder base class for audio in an Ogg container
                             -------------------
    begin                : Thu Jan 03 2013
    copyright            : (C) 2013 by Thomas Eschenbacher
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

#ifndef _OGG_SUB_ENCODER_H_
#define _OGG_SUB_ENCODER_H_

#include "config.h"

class QIODevice;
class QWidget;

namespace Kwave
{
    class FileInfo;
    class MultiWriter;

    class OggSubEncoder
    {
    public:
	/** Destructor */
	virtual ~OggSubEncoder() {}

	/**
	 * parse the header of the stream and initialize the decoder
	 * @param widget a QWidget to be used as parent for error messages
	 * @param info reference to a FileInfo to fill
	 * @param src MultiTrackReader used as source of the audio data
	 * @return true if succeeded, false if failed
	 */
	virtual bool open(QWidget *widget, const Kwave::FileInfo &info,
	                  Kwave::MultiTrackReader &src) = 0;

	/**
	 * write the header information
	 * @param dst a QIODevice that receives the raw data
	 * @return true if succeeded, false if failed
	 */
	virtual bool writeHeader(QIODevice &dst) = 0;

	/**
	 * encode received ogg data
	 * @param src MultiTrackReader used as source of the audio data
	 * @param dst a QIODevice that receives the raw data
	 * @return true if succeeded, false if failed
	 */
	virtual bool encode(Kwave::MultiTrackReader &src,
	                    QIODevice &dst) = 0;

	/**
	 * finish the decoding, last chance to fix up some file info
	 */
	virtual void close() = 0;

    };
}

#endif /* _OGG_SUB_ENCODER_H_ */

//***************************************************************************
//***************************************************************************
