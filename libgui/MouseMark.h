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

#ifndef _MOUSE_MARK_H_
#define _MOUSE_MARK_H_

#include "config.h"

/**
 * @class MouseMark
 * Simple class that can be used whenever the user selects something
 * with the mouse. Note that the coordinates within this class are
 * not transformed from screen pixels to the user's favorite
 * coordinate system. This must be done outside of here.
 */
class MouseMark
{

public:

    /** Constructor */
    MouseMark();

    /** Destructor */
    virtual ~MouseMark()
    {
    }

    /**
     * Sets the selection to a new range.
     * @param l start position
     * @param r end position
     */
    void set(unsigned int l, unsigned int r);

    /**
     * Update the last known position of the mouse. This should be
     * used for continuous update of the selection during mouse
     * movement.
     * @param x new last position
     */
    void update(unsigned int x);

    /**
     * Re-enters the selection process at a new position. The last
     * position will be set to the left or the right margin, depending
     * on which side is nearer.
     */
    void grep(unsigned int x);

    /**
     * Returns the left border of the selection.
     */
    unsigned int left();

    /**
     * Returns the right border of the selection.
     */
    unsigned int right();

    /**
     * Returns the lenght of the selection
     */
    inline unsigned int length() {
	return right() - left() + 1;
    }

private:
    /** initial position of the mouse */
    unsigned int initial;

    /** last known position of the mouse */
    unsigned int last;
};

//***********************************************************
#endif /*_MOUSE_MARK_H_ */
