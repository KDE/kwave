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

#include <qobject.h>

class Selection: public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param offset index of the first item
     * @param length number of items
     */
    Selection(unsigned int offset, unsigned int length)
	:QObject(), m_offset(offset), m_length(length)
    { };

    /** Destructor */
    virtual ~Selection() {};

    /**
     * Sets a new offset and length.
     * @param offset index of the first item
     * @param length number of items
     */
    void select(unsigned int offset, unsigned int length) {
    	m_offset = offset;
    	m_length = length;
    };

    /** Returns the index of the first selected item. */
    inline const unsigned int offset() {
	return m_offset;
    };

    /** Returns the number of selected items. */
    inline const unsigned int length() {
	return m_length;
    };

    /** Equal to offset(). */
    inline const unsigned int first() {
	return offset();
    };

    /** Returns the index of the last selected item. */
    inline const unsigned int last() {
	return m_offset + (m_length ? (m_length-1) : 0);
    };

private:
    /** index of the first selected item */
    unsigned int m_offset;

    /** number of items */
    unsigned int m_length;
};

#endif /* _SELECTION_H_ */
