/*************************************************************************
         StreamObject.h  -  base class with a generic sample source/sink
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

#ifndef _STREAM_OBJECT_H_
#define _STREAM_OBJECT_H_

#include "config.h"

#include <QtCore/QObject>
#include <QtCore/QMutex>

#include <kdemacros.h>

class QString;
class QVariant;

namespace Kwave
{

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
	 * Returns the default number of tracks that the source provides
	 * @return number of tracks, default is 1
	 */
	virtual unsigned int tracks() const { return 1; }

	/**
	 * Returns the source that corresponds to one specific track
	 * if the object has multiple tracks. For single-track objects
	 * it returns "this" for the first index and 0 for all others
	 * @param track index of the track
	 * @return a stream object or NULL
	 */
	virtual Kwave::StreamObject * operator [] (unsigned int track)
	{
	    return (track == 0) ? this : 0;
	}

	/**
	 * Returns the number of tracks of a input or output port.
	 * Can be overwritten for objects that have a different count
	 * of inputs and outputs.
	 * @param port name of the port (name of signal or slot)
	 * @return number of tracks of a input or output, default is
	 *         the same as tracks()
	 */
	virtual unsigned int tracksOfPort(const QString &port) const
	{
	    Q_UNUSED(port);
	    return tracks();
	}

	/**
	 * Returns an indexed port, identified by name
	 * @param port name of the port (name of signal or slot)
	 * @param track index of the track
	 */
	virtual Kwave::StreamObject *port(const QString &port,
	                                  unsigned int track)
	{
	    Q_UNUSED(port);
	    return (*this)[track];
	}

	/**
	 * Returns the block size used for producing data.
	 * @return currently 32k [samples]
	 */
	virtual unsigned int blockSize() const;

	/**
	 * Sets an attribute of a Kwave::StreamObject.
	 * @param attribute name of the attribute, with the signature of
	 *        a Qt SLOT(\<name\>(QVariant value))
	 * @param value the new value of the attribute, stored in a QVariant
	 */
	void setAttribute(const QString &attribute, const QVariant &value);

	/**
	 * Switch interactive mode on or off. In interactive mode we
	 * use a smaller block size for creating objects to get better
	 * response time to parameter changes. In non-interactive mode
	 * the block size is higher for better performance.
	 */
	static void setInteractive(bool interactive);

    signals:

	/**
	 * Emitted by setAttribute and connected to the corresponding
	 * slot.
	 */
	void attributeChanged(const QVariant value);

    private:

	/** Mutex for locking access to setAttribute (recursive) */
	QMutex m_lock_set_attribute;

	/** interactive mode: if enabled, use smaller block size */
	static bool m_interactive;

    };
}

#endif /* _STREAM_OBJECT_H_ */

//***************************************************************************
//***************************************************************************
