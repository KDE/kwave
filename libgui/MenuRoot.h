/***************************************************************************
                          MenuRoot.h  -  description
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

#ifndef _MENU_ROOT_H_
#define _MENU_ROOT_H_ 1

#include "MenuNode.h"

class KMenuBar;

/**
 * This is the class for the root of a Menu (e.g. a MenuBar) that contains
 * all toplevel menues of a menu hierarchy.
 * @author Thomas Eschenbacher
 */
class MenuRoot : public MenuNode
{
  Q_OBJECT

public: // Public methods

    /**
     * Constructor.
     * @param bar reference to a KMenuBar
     */
    MenuRoot(KMenuBar &bar);

    /**
     * Returns the positional index of a child node, identified by
     * it's menu id.
     * @param menu id of the child
     * @return index [0..n] or -1 f not found
     */
    virtual int getChildIndex(int id);

    /**
     * Inserts a new branch node into the menu structure. The new node
     * normally is (derived from) MenuToplevel.
     * @param name non-localized name of the node
     * @param command the command template used for creating commands of
     *                submenus (leafes) that don't have an own command
     *                but contain data for their parent's command.
     *                Should contain a %s that will be replaced by some
     *                data from a child entry. (this is used for
     *                menus with data selection lists like "recent files)
     *                If not used, pass 0.
     * @param key bitmask of the keyboard shortcut (see "qkeycode.h"),
     *            0 if unused
     * @param uid unique id string (might be 0)
     * @param index the positional index within the parent menu, starting
     *              from 0 or -1 for appending (optional, default=-1)
     * @return pointer to the new branch node
     */
    virtual MenuNode *insertBranch(char *name, char *command, int key,
                                   char *uid, int index=-1);

    /**
     * Inserts a new leaf node into the menu structure. The new node
     * normally is (derived from) MenuItem.
     * @param name non-localized name of the node
     * @param command the command to be sent when the node is
     *                selected (might be 0)
     * @param key bitmask of the keyboard shortcut (see "qkeycode.h"),
     *            0 if unused
     * @param uid unique id string (might be 0)
     * @param index the positional index within the parent menu, starting
     *              from 0 or -1 for appending. Optional, default=-1
     * @return pointer to the new leaf node
     */
    virtual MenuNode *insertLeaf(char *name, char *command,
                                 int key, char *uid,
                                 int index=-1);

    /**
     * Removes a child node of the curren node. If the child
     * was not found or is already removed this does nothing.
     * @param child pointer to the child node
     */
    virtual void removeChild(MenuNode *child);

    /**
     * Informs the node that the enabled state of a child node
     * might have changed.
     * @param id menu id of the child node
     * @param enable true if the item has been enabled, false if disabled
     */
    virtual void actionChildEnableChanged(int id, bool enable);

private: // Private attributes

  /** reference to a KMenuBar */
  KMenuBar &menu_bar;

};

#endif // _MENU_ROOT_H_
