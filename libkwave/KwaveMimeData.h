/***************************************************************************
        KwaveMimeData.h  -  mime data container for Kwave's audio data
			     -------------------
    begin                : Oct 04 2008
    copyright            : (C) 2008 by Thomas Eschenbacher
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

#ifndef _KWAVE_MIME_DATA_H_
#define _KWAVE_MIME_DATA_H_

#include "config.h"

#include <QByteArray>
#include <QMimeData>
#include <QObject>

#include <kdemacros.h>

class QMimeSource;
class QWidget;
class MultiTrackReader;
class FileInfo;
class SignalManager;

namespace Kwave {

    class KDE_EXPORT MimeData: public QMimeData
    {
	Q_OBJECT
	public:
	    /** Constructor */
	    MimeData();

	    /** Destructor */
	    virtual ~MimeData();

	    /**
	     * Encodes wave data received from a MultiTrackReader into a byte
	     * array that is compatible with the format of a wav file.
	     * @param widget the widget used for displaying error messages
	     * @param src source of the samples
	     * @param info information about the signal, sample rate,
	     *             resolution etc
	     * @return true if successful
	     */
	    virtual bool encode(QWidget *widget,
	                        MultiTrackReader &src,
	                        FileInfo &info);

	    /**
	     * Decodes the encoded byte data of the given mime source and
	     * initializes a MultiTrackReader.
	     * @param widget the widget used for displaying error messages
	     * @param e mime source
	     * @param sig signal that receives the mime data
	     * @return number of decoded samples if successful, zero if failed
	     */
	    static unsigned int decode(QWidget *widget, const QMimeSource *e,
	                               SignalManager &sig, unsigned int pos);

	private:

	    /** simple array for storage of the wave data */
	    QByteArray m_data;
    };
}

#endif /* _KWAVE_MIME_DATA_H_ */
