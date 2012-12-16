/***************************************************************************
			  MenuGroup.h  -  controls a group of menu nodes
			     -------------------
    begin                : Mon Jan 10 2000
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

#ifndef _MENU_GROUP_H_
#define _MENU_GROUP_H_

#include "config.h"
#include "libgui/MenuNode.h"

namespace Kwave
{

    /**
     * A MenuGroup controls a group of menu nodes (items, submenus).
     * @author Thomas Eschenbacher
     */
    class MenuGroup : public Kwave::MenuNode
    {
	Q_OBJECT

    public:

	/**
	 * Constructor.
	 * @param parent pointer to the group's parent (might be 0)
	 * @param name the unique name of the group
	 */
	MenuGroup(Kwave::MenuNode *parent, const QString &name);

	/**
	 * Destructor. cleans up.
	 * @see #clear()
	 */
	virtual ~MenuGroup();

	/**
	 * Enables/disables all members of the group.
	 * @param enable true to enable the item, false to disable
	 */
	virtual void setEnabled(bool enable);

	/**
	 * Resets all checkmarks of the group members except the one member
	 * that will get the new selected one. If no new member id is given
	 * no member will get selected. This method is useful for making
	 * exclusive selections of menu items.
	 * @param uid the unique id string of the member to be selected or 0
	 */
	virtual void selectItem(const QString &uid);

	/**
	 * Deregisteres all child nodes from us and removes them from
	 * our internal list of child nodes.
	 */
	virtual void clear();

    };
}

#endif // _MENU_GROUP_H_

//***************************************************************************
//***************************************************************************
