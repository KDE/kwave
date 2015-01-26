/***************************************************************************
             Selection.h - Simple class for a selection
			     -------------------
    begin                : Sun May 06 2001
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#ifndef _SELECTION_H_
#define _SELECTION_H_

#include "config.h"

#include <QtCore/QObject>

#include <kdemacros.h>

#include "libkwave/Sample.h"

namespace Kwave
{

    class KDE_EXPORT Selection: public QObject
    {
	Q_OBJECT
    public:
	/**
	 * constructor.
	 * @param offset index of the first item
	 * @param length number of items
	 */
	Selection(sample_index_t offset, sample_index_t length);

	/** copy constructor */
	Selection(const Selection &other);

	/** Destructor */
	virtual ~Selection();

	/**
	 * Sets a new offset and length.
	 * @param offset index of the first item
	 * @param length number of items
	 */
	void select(sample_index_t offset, sample_index_t length);

	/** Clears the selection (0 samples at offset 0) */
	inline void clear() {
	    select(0, 0);
	}

	/** Returns the index of the first selected item. */
	inline sample_index_t offset() const {
	    return m_offset;
	}

	/** Returns the number of selected items. */
	inline sample_index_t length() const {
	    return m_length;
	}

	/** Equal to offset(). */
	inline sample_index_t first() const {
	    return offset();
	}

	/** Returns the index of the last selected item. */
	inline sample_index_t last() const {
	    return m_offset + (m_length ? (m_length-1) : 0);
	}

	/** compare operator */
	bool operator == (const Selection &other) const {
	    return ((m_offset == other.offset()) &&
		    (m_length == other.length()));
	}

	/** Assignment operator */
	Selection & operator = (const Selection &source) {
	    m_offset = source.offset();
	    m_length = source.length();
	    return *this;
	}

    signals:

	/**
	 * Emits a change in the selected range.
	 * @param offset index of the first selected items
	 * @param length number of selected items
	 */
	void changed(sample_index_t offset, sample_index_t length);

    private:
	/** index of the first selected item */
	sample_index_t m_offset;

	/** number of items */
	sample_index_t m_length;
    };
}

#endif /* _SELECTION_H_ */

//***************************************************************************
//***************************************************************************
