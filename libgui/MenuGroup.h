/***************************************************************************
                          MenuGroup.h  -  description
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
#define _MENU_GROUP_H_ 1

#include "MenuNode.h"

/**
 * A MenuGroup controls a group of menu nodes (items, submenus).
 * @author Thomas Eschenbacher
 */
class MenuGroup : public MenuNode
{
    Q_OBJECT

public: // Public methods

    MenuGroup(MenuNode *parent, char *name);

    /**
     * Registers a node as a child of the current node.
     * @param child pointer to the child node
     * @return the id of the node
     */
    virtual int registerChild(MenuNode *child);

    /**
     * Removes a child node of the curren node. If the child
     * was not found or is already removed this does nothing.
     * @param child pointer to the child node
     */
    virtual void removeChild(MenuNode *child);

    /**
     * Enables/disables all members of the group.
     * @param enable true to enable the item, false to disable
     */
    virtual void setEnabled(bool enable);

    /**
     * Resets all checkmarks of the group members except from one member
     * that will get the new selected one. If no new member id is given
     * no member will get selected. This method is useful for making
     * exclusive selections of menu items.
     * @param uid the unique id string of the member to be selected or 0
     */
    virtual void selectItem(const char *uid);

};

#endif // _MENU_GROUP_H_
