/***************************************************************************
          ViewManager.h  -  abstract interface for managing signal views
                            -------------------
    begin                : Sat Mar 27 2010
    copyright            : (C) 2010 by Thomas Eschenbacher
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

#ifndef VIEW_MANAGER_H
#define VIEW_MANAGER_H

class QWidget;

namespace Kwave
{

    class SignalView;

    /**
     * Abstract interface for registering a SignalView in the main widget
     */
    class KDE_EXPORT ViewManager
    {
    public:
	/** Destructor */
	virtual ~ViewManager() {}

	/**
	 * Insert a new signal view into this widget (or the upper/lower
	 * dock area.
	 * @param view the signal view, must not be a null pointer
	 * @param controls a widget with controls, optionally, can be null
	 */
	virtual void insertView(Kwave::SignalView *view, QWidget *controls) = 0;
    };
}

#endif /* VIEW_MANAGER_H */

//***************************************************************************
//***************************************************************************
