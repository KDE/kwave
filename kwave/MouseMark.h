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

    /**
     * Sets the selection to a new range.
     * @param l start position
     * @param r end position
     */
    void set(int l, int r);

    /**
     * Update the last known position of the mouse. This should be
     * used for continuous update of the selection during mouse
     * movement.
     * @param x new last position
     */
    void update(int x);

    /**
     * Re-enters the selection process at a new position. The last
     * position will be set to the left or the right margin, depending
     * on which side is nearer.
     */
    void grep(int x);

    /**
     * Returns true if x is in the border range between
     * selected and unselected.
     * @param x position to be checked
     * @param tol tolerance
     * @return true if the position is in range
     */
    bool checkPosition(int x, int tol);

    /**
     * Returns the left border of the selection.
     */
    int left();

    /**
     * Returns the right border of the selection.
     */
    int right();

private:
    /** initial position of the mouse */
    int initial;

    /** last known position of the mouse */
    int last;
}
;
//***********************************************************
#endif /*_MOUSE_MARK_H_ */
