/***************************************************************************
                          MenuItem.h  -  selectable and checkable menu item
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

#ifndef _MENU_ITEM_H_
#define _MENU_ITEM_H_

#include <libkwave/String.h>

#include "MenuNode.h"

/**
 * Base class for entries in a Menu. It is normally owned by a toplevel
 * menu or a submenu.
 * @author Thomas Eschenbacher
 */
class MenuItem : public MenuNode {
  Q_OBJECT

public: // Public methods

    /**
     * Constructor.
     * @param pointer to the node's parent (might be 0)
     * @param name the non-localized name of the node
     * @param command the command to be sent when the node is
     *                selected (optional, default=0)
     * @param key bitmask of the keyboard shortcut (see "qkeycode.h"),
     *            (optional, default=0)
     * @param uid unique id string (optional, default=0)
     */
    MenuItem(MenuNode *parent, char *name, char *command=0,
             int key=0, char *uid=0);

    /** virtual destructor */
    virtual ~MenuItem();

    /**
     * Called to notify the item that it has been selected.
     */
    virtual void actionSelected();

    /**
     * Positional index of the iten in the parent menu.
     * @return index [0..n] or -1 if no parent
     */
    virtual int getIndex();

    /**
     * Handles/interpretes special menu commands.
     * @param command name of a menu node or command
     * @return true if the name was recognized as a command and handled
     */
    virtual bool specialCommand(const char *command);

    /**
     * Enables/disabled checking/selecting the item
     * @param checkable true to enable checking, false for disabling
     */
    virtual void setCheckable(bool checkable);

    /** Returns true if the node is checkable/selectable */
    virtual bool isCheckable();

    /**
     * Sets/removes the checkmark from the current menu item.
     * @param check true to set the mark, false to remove
     */
    virtual void setChecked(bool check);

private:

    /** true if the item can be selected/checked (default=false) */
    bool checkable;

    /**
     * name of a group for exclusive selection
     * (optional, set by special command, default=0)
     */
    const char *exclusive_group;

};

#endif // _MENU_ITEM_H_
