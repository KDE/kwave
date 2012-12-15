/***************************************************************************
             TypesMap.h  -  Map with index, data, command, description
			     -------------------
    begin                : Feb 05 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef _TYPES_MAP_H_
#define _TYPES_MAP_H_

#include "config.h"

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <klocale.h>

#include "libkwave/String.h"
#include "libkwave/Triple.h"

namespace Kwave
{

    template <class IDX, class DATA> class TypesMap
    {
    private:
	typedef QMap <IDX, Kwave::Triple <DATA, QString, QString> > TripleMap;

    public:
	/**
	 * Default constructor. Must be overwritten to initialize
	 * the list with useful values.
	 */
	TypesMap()
	    :m_list()
	{
	}

	/** Destructor */
	virtual ~TypesMap()
	{
	    m_list.clear();
	}

	/**
	 * This function is abstract and must be overwritten to
	 * initially fill the map if it was empty.
	 */
	virtual void fill() = 0;

	/**
	 * Appends a new type into the map.
	 * @param index unique index within the map
	 * @param data the data associated with the entry
	 * @param name string representation of the type, for
	 *             internal usage in Kwave commands.
	 * @param description text for the user interface
	 */
	virtual void append(IDX index, DATA data,
	    const QString &name, const QString &description)
	{
	    Kwave::Triple<DATA, QString, QString>
		triple(data, name, description);
	    m_list.insert(index, triple);
	}

	/** Returns the number of types. */
	inline unsigned int count() const {
	    return m_list.count();
	}

	/**
	 * Try to find the type from the data. If the data item is not found,
	 * the return value is the default value of the type (casted from 0).
	 */
	IDX findFromData(const DATA &data) const
	{
	    IDX it = IDX(0);
	    unsigned int cnt = count();
	    while (cnt--) {
		if (m_list[it].first() == data) return it;
		++it;
	    }
	    return IDX(0);
	}

	/**
	 * Try to find the type from a name. If the name is not found,
	 * the return value is the default value of the type (casted from 0).
	 */
	IDX findFromName(const QString &name) const
	{
	    IDX it = IDX(0);
	    unsigned int cnt = count();
	    while (cnt--) {
		if (m_list[it].second() == name) return it;
		++it;
	    }
	    return IDX(0);
	}

	/**
	 * Try to find the type from an optionally localized description.
	 * If the name is not found, the return value is the default
	 * value of the type (casted from 0).
	 * @param description the description to be searched for
	 * @param localized if true, the localized description will be
	 *        searched
	 */
	IDX findFromDescription(
	    const QString &description, bool localized) const
	{
	    IDX it = IDX(0);
	    QString dcr = (localized) ? i18n(description.toUtf8()) : description;
	    unsigned int cnt = count();
	    while (cnt--) {
		if (localized) {
		    if (m_list[it].third() == description)
			return it;
		} else {
		    if (i18n(m_list[it].third().toUtf8()) ==
			i18n(description.toUtf8()))
			return it;
		}
		++it;
	    }
	    return IDX(0);
	}

	/** Returns the data item of a type. */
	const DATA &data(IDX type) const
	{
	    return m_list[type].first();
	}

	/** Returns the name of a type. */
	const QString name(IDX type) const
	{
	    return m_list[type].second();
	}

	/**
	 * Returns the description of a type.
	 * @param type index of the type
	 * @param localized if true, the returned description is localized
	 */
	QString description(IDX type, bool localized) const
	{
	    QString s(m_list[type].third());
	    return (localized) ? i18n(s.toAscii()) : s;
	}

	/**
	 * Returns a string list with all names,
	 */
	QStringList allNames() const
	{
	    IDX it = IDX(0);
	    unsigned int cnt = count();
	    QStringList names;
	    while (cnt--) {
		names.append(m_list[it].second());
		++it;
	    }
	    return names;
	}

	/**
	 * Returns a list with all keys
	 */
	QList<IDX> allKeys() const
	{
	    IDX it = IDX(0);
	    unsigned int cnt = count();
	    QList<IDX> keys;
	    while (cnt--) {
		IDX key = it;
		keys.append(key);
		++it;
	    }
	    return keys;
	}

    private:

	/** map with index and triples of data, name and description */
	TripleMap m_list;

    };
}

#endif /* _TYPES_MAP_H_ */

//***************************************************************************
//***************************************************************************
