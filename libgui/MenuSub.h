/***************************************************************************
			  MenuSub.h  -  submenu
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

#ifndef _MENU_SUB_H_
#define _MENU_SUB_H_ 1

#include "MenuItem.h"

class QPixmap;
class QPopupMenu;

/**
 * This is the class for submenu entries in a Menu. It is normally owned by a
 * root menu node, a toplevel menu or an other submenu.
 * @author Thomas Eschenbacher
 */
class MenuSub : public MenuItem
{
    Q_OBJECT

public:
    /**
     * Constructor.
     * @param parent pointer to the node's parent (might be 0)
     * @param name the non-localized name of the submenu
     * @param command the command to be sent when the submenu is
     *                selected (optional, default=0)
     * @param key bitmask of the keyboard shortcut (see "qkeycode.h"),
     *            (optional, default=0)
     * @param uid unique id string (optional, default=0)
     */
    MenuSub(MenuNode *parent, const QString &name, const QString &command = 0,
	    int key = 0, const QString &uid = 0);

    /**
     * Returns the positional index of a child node, identified by
     * it's menu id.
     * @param menu id of the child
     * @return index [0..n] or -1 f not found
     */
    virtual int getChildIndex(int id);

    /**
     * Always returns true, as the nodes of this type are branches.
     */
    virtual bool isBranch() {
	return true;
    };

    /**
     * Inserts a new branch node under the submenu. The new node
     * normally is (derived from) MenuSub.
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
    virtual MenuNode *insertBranch(const QString &name,
	const QString &command, int key, const QString &uid,
	int index = -1);

    /**
     * Inserts a new leaf node under the submenu. The new node
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
    virtual MenuNode *insertLeaf(const QString &name, const QString &command,
				 int key, const QString &uid,
				 int index = -1);

    /**
     * Returns the internally handled QPopupMenu
     */
    virtual QPopupMenu *getPopupMenu();

    /**
     * Removes a child node of the curren node. If the child
     * was not found or is already removed this does nothing.
     * @param child pointer to the child node
     */
    virtual void removeChild(MenuNode *child);

    /**
     * Handles/interpretes special menu commands.
     * @param command name of a menu node or command
     * @return true if the name was recognized as a command and handled
     */
    virtual bool specialCommand(const QString &command);

    /**
     * Sets a new icon of a child node.
     * @param id the node's menu id
     * @param icon reference to the QPixmap with the icon
     */
    virtual void setItemIcon(int id, const QPixmap &icon);

    /**
     * Sets or removes the checkmark from a menu item.
     * @param id the item's menu id
     * @param check true to set the mark, false to remove
     */
    virtual void setItemChecked(int id, bool check);

    /**
     * Informs the submenu that the enabled state of a child node
     * might have changed.
     * @param id menu id of the child node
     * @param enable true if the item has been enabled, false if disabled
     */
    virtual void actionChildEnableChanged(int id, bool enable);

public slots:

    /**
     * will be called if an item of the submenu is selected
     * @param id the numeric id of the selected item (child)
     */
    void slotSelected(int id);

private:

    /** the QPopupMenu that is controlled */
    QPopupMenu *menu;
};

#endif /* _MENU_SUB_H_ */
