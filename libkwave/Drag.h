/***************************************************************************
                 Drag.h  -  Drag&Drop container for Kwave's audio data
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

#ifndef DRAG_H
#define DRAG_H

#include "config.h"

#include <QtGlobal>
#include <QByteArray>
#include <QDrag>
#include <QObject>
#include <QString>

#include "libkwave/Sample.h"

class QMimeData;
class QWidget;

namespace Kwave
{

    class MetaDataList;
    class MultiTrackReader;
    class SignalManager;

    /**
     * Simple class for drag & drop of wav data.
     * @todo the current storage mechanism is straight-forward and stupid, it
     *       should be extended to use virtual memory
     */
    class Q_DECL_EXPORT Drag: public QDrag
    {
	Q_OBJECT

    public:
	/**
	 * Constructor
	 * @see QDragObject
	 */
	explicit Drag(QWidget *dragSource = 0);

	/** Destructor */
	virtual ~Drag();

	/**
	 * Encodes wave data received from a MultiTrackReader into a byte
	 * array that is compatible with the format of a wav file.
	 * @param widget the widget used for displaying error messages
	 * @param src source of the samples
	 * @param meta_data information about the signal, sample rate,
	 *                  resolution and other meta data
	 * @return true if successful
	 */
	bool encode(QWidget *widget, Kwave::MultiTrackReader &src,
	            const Kwave::MetaDataList &meta_data);

	/** Returns true if the mime type of the given source can be decoded */
	static bool canDecode(const QMimeData *data);

	/**
	 * Decodes the encoded byte data of the given mime source and
	 * initializes a MultiTrackReader.
	 * @param widget the widget used for displaying error messages
	 * @param e mime source
	 * @param sig signal that receives the mime data
	 * @param pos position within the signal where to insert the data
	 * @return number of decoded samples if successful, zero if failed
	 */
	static sample_index_t decode(QWidget *widget, const QMimeData *e,
	                             Kwave::SignalManager &sig,
	                             sample_index_t pos);

    };
}

#endif /* DRAG_H */

//***************************************************************************
//***************************************************************************
