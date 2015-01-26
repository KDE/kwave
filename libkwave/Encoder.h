/***************************************************************************
              Encoder.h  -  abstract base class of all encoders
			     -------------------
    begin                : Mar 10 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#ifndef _ENCODER_H_
#define _ENCODER_H_

#include "config.h"
#include <QtCore/QList>
#include <QtCore/QObject>

#include <kdemacros.h>

#include "libkwave/CodecBase.h"
#include "libkwave/FileInfo.h"

class QIODevice;
class QWidget;

namespace Kwave
{

    class MultiTrackReader;

    class KDE_EXPORT Encoder: public QObject, public Kwave::CodecBase
    {
	Q_OBJECT
    public:
	/** Constructor */
	Encoder();

	/** Destructor */
	virtual ~Encoder() {}

	/** Returns a new instance of the encoder */
	virtual Encoder *instance() = 0;

	/**
	 * Encodes a signal into a stream of bytes.
	 * @param widget a widget that can be used for displaying
	 *        message boxes or dialogs
	 * @param src MultiTrackReader used as source of the audio data
	 * @param dst file or other source to receive a stream of bytes
	 * @param meta_data meta data of the file to save
	 * @return true if succeeded, false on errors
	 */
	virtual bool encode(QWidget *widget,
	                    Kwave::MultiTrackReader &src,
	                    QIODevice &dst,
	                    const Kwave::MetaDataList &meta_data) = 0;

	/** Returns a list of supported file properties */
	virtual QList<Kwave::FileProperty> supportedProperties() {
	    QList<Kwave::FileProperty> empty;
	    return empty;
	}

    };
}

#endif /* _ENCODER_H_ */

//***************************************************************************
//***************************************************************************
