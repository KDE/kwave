/*************************************************************************
       WavPropertyMap.h  -  map for translating properties to chunk names
                             -------------------
    begin                : Sat Jul 06 2002
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

#ifndef _WAV_PROPERTY_MAP_H_
#define _WAV_PROPERTY_MAP_H_

#include "config.h"

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QString>

#include "libkwave/FileInfo.h"

namespace Kwave
{
    class WavPropertyMap
	:protected QList< QPair<Kwave::FileProperty, QByteArray> >
    {
    public:
	/** Default constructor, with initializing */
	WavPropertyMap();

	/** Destructor */
	virtual ~WavPropertyMap() {};

	/**
	 * Returns the chunk name of a property or an empty string
	 * if nothing found (reverse lookup).
	 */
	QByteArray findProperty(const Kwave::FileProperty property) const;

	/** Returns true if the map contains a given property */
	bool containsProperty(const Kwave::FileProperty property) const;

	/**
	 * insert a new property / chunk mapping
	 *
	 * @param property a Kwave FileProperty
	 * @param chunk a 4-byte chunk name
	 */
	void insert(const Kwave::FileProperty property, const QByteArray &chunk);

	/**
	 * returns true if a given chunk is in the list
	 *
	 * @param chunk a 4-byte chunk name
	 * @return true if found, false if not
	 */
	bool containsChunk(const QByteArray &chunk) const;

	/** returns a list of all known chunks */
	QList<QByteArray> chunks() const;

	/**
	 * Returns the first FileProperty that matches a given chunk
	 *
	 * @param chunk a 4-byte chunk name
	 * @return a FileProperty
	 */
	Kwave::FileProperty property(const QByteArray &chunk) const;

	/** Returns a list with all supported properties */
	QList<Kwave::FileProperty> properties() const;

    private:

	typedef QPair<Kwave::FileProperty, QByteArray> Pair;

    };
}

#endif /* _WAV_PROPERTY_MAP_H_ */

//***************************************************************************
//***************************************************************************
