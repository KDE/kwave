/*************************************************************************
         AsciiEncoder.h  -  encoder for ASCII data
                             -------------------
    begin                : Sun Nov 26 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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

#ifndef _ASCII_ENCODER_H_
#define _ASCII_ENCODER_H_

#include "config.h"

#include <QtCore/QList>
#include <QtCore/QTextStream>

#include "libkwave/Encoder.h"

class QWidget;

namespace Kwave
{
    class MultiTrackReader;
    class MetaDataList;

    class AsciiEncoder: public Kwave::Encoder
    {
    public:
	/** Constructor */
	AsciiEncoder();

	/** Destructor */
	virtual ~AsciiEncoder();

	/** Returns a new instance of the encoder */
	virtual Kwave::Encoder *instance();

	/**
	 * Encodes a signal into a stream of bytes.
	 * @param widget a widget that can be used for displaying
	 *        message boxes or dialogs
	 * @param src MultiTrackReader used as source of the audio data
	 * @param dst file or other source to receive a stream of bytes
	 * @param meta_data meta information about the file to be saved
	 * @return true if succeeded, false on errors
	 */
	virtual bool encode(QWidget *widget, Kwave::MultiTrackReader &src,
	                    QIODevice &dst,
	                    const Kwave::MetaDataList &meta_data);

	/** Returns a list of supported file properties */
	virtual QList<Kwave::FileProperty> supportedProperties();

    private:

	/** pointer to the QIODevice for storing, used while encoding */
	QTextStream m_dst;

    };
}

#endif /* _ASCII_ENCODER_H_ */

//***************************************************************************
//***************************************************************************
