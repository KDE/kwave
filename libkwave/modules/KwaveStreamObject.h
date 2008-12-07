/*************************************************************************
    KwaveStreamObject.h  -  base class with a generic sample source/sink
                             -------------------
    begin                : Thu Nov 01 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
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

#ifndef _KWAVE_STREAM_OBJECT_H_
#define _KWAVE_STREAM_OBJECT_H_

#include "config.h"

#include <QObject>
#include <QMutex>

#include <kdemacros.h>

class QString;
class QVariant;

namespace Kwave {
    class KDE_EXPORT StreamObject: public QObject
    {
	Q_OBJECT
    public:
	/**
	 * Constructor
	 *
	 * @param parent a parent object, passed to QObject (optional)
	 */
	StreamObject(QObject *parent = 0);

	/** Destructor */
	virtual ~StreamObject();

	/**
	 * Returns the number of tracks that the source provides
	 * @return number of tracks, default is 1
	 */
	virtual unsigned int tracks() const { return 1; }

	/**
	 * Returns the source that corresponds to one specific track
	 * if the object has multiple tracks. For single-track objects
	 * it returns "this" for the first index and 0 for all others
	 */
	virtual Kwave::StreamObject * operator [] (unsigned int track)
	{
	    return (track == 0) ? this : 0;
	}

	/**
	 * Returns the block size used for producing data.
	 * @return currently 1024 [samples]
	 */
	virtual unsigned int blockSize() const {
	    return 1024;
	}

	/**
	 * Sets an attribute of a Kwave::StreamObject.
	 * @param attribute name of the attribute, with the signature of
	 *        a Qt SLOT(<name>(QVariant value))
	 * @param value the new value of the attribute, stored in a QVariant
	 */
	void setAttribute(const QString &attribute, const QVariant &value);

    signals:

	/**
	 * Emitted by setAttribute and connected to the corresponding
	 * slot.
	 */
	void attributeChanged(const QVariant value);

    private:

	/** Mutex for locking access to setAttribute (recursive) */
	QMutex m_lock_set_attribute;

    };
}

#endif /* _KWAVE_STREAM_OBJECT_H_ */
