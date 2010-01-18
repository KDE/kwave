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

#include <QObject>

#include <kdemacros.h>

class KDE_EXPORT Selection: public QObject
{
    Q_OBJECT
public:
    /**
     * constructor.
     * @param offset index of the first item
     * @param length number of items
     */
    Selection(unsigned int offset, unsigned int length);

    /** copy constructor */
    Selection(const Selection &other);

    /** Destructor */
    virtual ~Selection();

    /**
     * Sets a new offset and length.
     * @param offset index of the first item
     * @param length number of items
     */
    void select(unsigned int offset, unsigned int length) {
	if ((offset == m_offset) && (length == m_length))
	    return;
	m_offset = offset;
	m_length = length;
	emit changed(m_offset, m_length);
    }

    /** Clears the selection (0 samples at offset 0) */
    inline void clear() {
	select(0, 0);
    }

    /** Returns the index of the first selected item. */
    inline unsigned int offset() const {
	return m_offset;
    }

    /** Returns the number of selected items. */
    inline unsigned int length() const {
	return m_length;
    }

    /** Equal to offset(). */
    inline unsigned int first() const {
	return offset();
    }

    /** Returns the index of the last selected item. */
    inline unsigned int last() const {
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
    void changed(unsigned int offset, unsigned int length);

private:
    /** index of the first selected item */
    unsigned int m_offset;

    /** number of items */
    unsigned int m_length;
};

#endif /* _SELECTION_H_ */
