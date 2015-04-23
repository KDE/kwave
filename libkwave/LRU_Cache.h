/***************************************************************************
            LRU_Cache.h  -  template class for a LRU cache
			     -------------------
    begin                : Thu Jun 18 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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

#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include "config.h"

#include <QtCore/QLinkedList>
#include <QtCore/QMutableLinkedListIterator>
#include <QtCore/QPair>

namespace Kwave
{

    template<class IDX, class DATA> class LRU_Cache
        :public QLinkedList< QPair<IDX, DATA> >
    {
    private:
	/** internal typedef for the QPair */
	typedef QPair<IDX,DATA> Pair;

    public:
	/** Constructor */
	LRU_Cache()
	    :QLinkedList< Pair >()
	{
	}

	/** Destructor */
	virtual ~LRU_Cache()
	{
	}

	/** returns true if the cache is empty */
	bool isEmpty() const {
	    return QLinkedList<Pair>::isEmpty();
	}

	/** index operator, const */
	const DATA & operator [] (const IDX index) const
	{
	    foreach(const Pair &p, *this) {
		if (p.first == index) {
		    return p.second;
		}
	    }
	    return DATA();
	}

	/** index operator, non-const */
	DATA & operator [] (const IDX index)
	{
	    static DATA dummy;

	    if (!QLinkedList<Pair>::isEmpty()) {
		QMutableLinkedListIterator<Pair> it(*this);
		while (it.hasNext()) {
		    Pair &p = it.next();
		    if (p.first == index) {
			if (p.first != QLinkedList<Pair>::first().first) {
			    // found it -> move the entry to the start of the list
			    Pair pair = p;
			    it.remove();
			    insert(pair.first, pair.second);
// 			    qDebug("Kwave::MemoryManager[%9d] - reordering",
// 			           pair.first);
			}

			// get the newly entered entry again
			return QLinkedList<Pair>::first().second;
		    }
		}
	    }
	    return dummy;
	}

	/**
	 * checks whether an index is contained
	 * @param index the index (key) of the element to look up
	 * @return true if found, otherwise false
	 */
	bool contains(const IDX index) const
	{
	    foreach(const Pair &p, *this) {
		if (p.first == index)
		    return true;
	    }
	    return false;
	}

	/**
	 * remove an entry
	 * @param index the index (key) of the element to remove
	 */
	void remove(const IDX index)
	{
	    if (!QLinkedList<Pair>::isEmpty()) {
		QMutableLinkedListIterator<Pair> it(*this);
		while (it.hasNext()) {
		    Pair &p = it.next();
		    if (p.first == index) {
			it.remove();
			break;
		    }
		}
	    }
	}

	/**
	 * insert a new entry to the start of the list (newest)
	 * @param index the index (key) of the element to insert
	 * @param value the data of the element to insert
	 */
	void insert(const IDX index, DATA &value)
	{
	    QLinkedList<Pair>::prepend(Pair(index, value));
	}

	/** returns a list of keys */
	QList<IDX> keys() const
	{
	    QList<IDX> i;
	    foreach(const Pair &p, *this) {
		i << p.first;
	    }
	    return i;
	}

	/** returns a list of values */
	QList<DATA> values() const
	{
	    QList<DATA> v;
	    foreach(const Pair &p, *this) {
		v << p.second;
	    }
	    return v;
	}

    };

}

#endif /* LRU_CACHE_H */

//***************************************************************************
//***************************************************************************
