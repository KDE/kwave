/***************************************************************************
            KwaveDrag.h  -  Drag&Drop container for Kwave's audio data
			     -------------------
    begin                : Jan 24 2002
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

#ifndef _KWAVE_DRAG_H_
#define _KWAVE_DRAG_H_

#include <qcstring.h>
#include <qdragobject.h>
#include <qmime.h>
#include <qobject.h>

class QImage;
class QMimeSource;
class QWidget;

class MultiTrackReader;

class Q_EXPORT KwaveDrag: public QDragObject
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @see QDragObject
     */
    KwaveDrag(QWidget *dragSource = 0, const char *name = 0);

    /** Destructor */
    virtual ~KwaveDrag();

    /**
     * Returns one entry in the list of supported formats.
     * @param i index of the format [0...n]
     * @return name of the format or null
     */
    virtual const char *format(int i) const;

    virtual QByteArray encodedData(const char *format) const;

    /**
     * Encodes wave data received from a MultiTrackReader into a byte
     * array that is compatible with the format of a wav file.
     * @param rate sample rate of the signal [samples/sec]
     * @param bits resolution [bits per sample]
     * @param src source of the samples
     * @return true if successful
     */
    bool encode(unsigned int rate, unsigned int bits,
                MultiTrackReader &src);

    /** Returns true if the mime type of the given source can be decoded */
    static bool canDecode(const QMimeSource* e);

    /**
     * Decodes the encoded byte data of the given mime source and
     * initializes a MultiTrackReader.
     * @param rate receives the sample rate of the signal [samples/sec]
     * @param bits receives resolution [bits per sample]
     * @param src source with of the decoded sample stream
     */
    static bool decode(const QMimeSource *e, unsigned int &rate,
                       unsigned int &bits, MultiTrackReader &src);

private:

    /** simple array for storage of the wave data */
    QByteArray data;

};

#endif /* _KWAVE_DRAG_H_ */
