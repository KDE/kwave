/***************************************************************************
             MouseMark.h -  Handling of mouse selection
			     -------------------
    begin                : Sun Nov 12 2000
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

#ifndef MOUSE_MARK_H
#define MOUSE_MARK_H

#include "config.h"

#include "libkwave/Sample.h"

namespace Kwave
{
    /**
     * Simple class that can be used whenever the user selects something
     * with the mouse.
     */
    class MouseMark
    {

    public:

	/** Constructor */
	MouseMark();

	/** Destructor */
	virtual ~MouseMark();

	/**
	 * Sets the selection to a new range.
	 * @param l start position
	 * @param r end position
	 */
	void set(sample_index_t l, sample_index_t r);

	/**
	 * Update the last known position of the mouse. This should be
	 * used for continuous update of the selection during mouse
	 * movement.
	 * @param x new last position
	 */
	void update(sample_index_t x);

	/**
	 * Re-enters the selection process at a new position. The last
	 * position will be set to the left or the right margin, depending
	 * on which side is nearer.
	 */
	void grep(sample_index_t x);

	/**
	 * Returns the left border of the selection.
	 */
	sample_index_t left() const;

	/**
	 * Returns the right border of the selection.
	 */
	sample_index_t right() const;

	/**
	 * Returns the length of the selection
	 */
	inline sample_index_t length() const {
	    return right() - left() + 1;
	}

    private:
	/** initial position of the mouse */
	sample_index_t m_initial;

	/** last known position of the mouse */
	sample_index_t m_last;
    };
}

#endif /* MOUSE_MARK_H */

//***************************************************************************
//***************************************************************************
