/***************************************************************************
			  MenuNode.h  -  generic menu node type
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

#ifndef _MENU_NODE_H_
#define _MENU_NODE_H_

#include "config.h"
#include <qobject.h>
#include <qlist.h>
#include <qdict.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qstringlist.h>

class MenuNode;
class MenuSub;
class MenuGroup;

/**
 * Base class for the MenuRoot, MenuEntry, SubMenu and
 * the ToplevelMenu class.
 * @author Thomas Eschenbacher
 */
class MenuNode: public QObject
{
    Q_OBJECT

    friend class MenuSub;
    friend class MenuItem;

public:
    /**
     * Constructor.
     * @param parent pointer to the node's parent (might be 0)
     * @param name the non-localized name of the node
     * @param command the command to be sent when the node is
     *                selected (optional, default=0)
     * @param key bitmask of the keyboard shortcut (see "qkeycode.h"),
     *            (optional, default=0)
     * @param uid unique id string (optional, default=0)
     */
    MenuNode(MenuNode *parent, const QString &name,
	const QString &command = 0, int key = 0, const QString &uid = 0);

    /**
     * Destructor. Clears the menu node and cleans up.
     * @see #clear()
     */
    virtual ~MenuNode();

    /**
     * Returns the (non-localized) name of the node.
     */
    inline const QString &getName() {
	return m_name;
    };

    /**
     * Returns the command of the node.
     */
    inline const QString &getCommand() {
	return m_command;
    };

    /**
     * Returns the menu id of the node.
     */
    inline int getId() {
	return m_id;
    };

    /**
     * Returns the unique id string of the node.
     */
    inline const QString &getUID() {
	return m_uid;
    };

    /**
     * Sets the unique id string of the node
     */
    void setUID(const QString &uid);

    /**
     * Returns the bitmask of the keyboard shortcut.
     */
    inline int getKey() {
	return m_key;
    };

    /**
     * Sets the bitmask of the keyboard shortcut.
     */
    virtual void setKey(int key) {
	m_key = key;
    };

    /**
     * Returns a reference to the menu node's icon.
     */
    virtual const QPixmap &getIcon();

    /**
     * Sets a new icon of a menu node.
     * @param icon QPixmap with the icon
     */
    virtual void setIcon(const QPixmap icon);

    /**
     * Sets a new icon of a menu node's child node
     * @param id the node's menu id
     * @param icon reference to the QPixmap with the icon
     */
    virtual void setItemIcon(int id, const QPixmap &icon);

    /**
     * Sets the menu id of a node.
     * @param id new menu id of the node
     */
    inline void setId(int id) {
	m_id = id;
    };

    /**
     * Positional index of the node in the parent node.
     * (overwritten in MenuItem and MenuSub)
     * @return index [0..n] or -1 if no parent
     */
    virtual int getIndex() {
	return -1;
    };

    /**
     * Returns the positional index of a child node, identified by
     * it's menu id.
     * @param menu id of the child
     * @return index [0..n] or -1 f not found
     */
    virtual int getChildIndex(int id);

    /**
     * Returns true if the node is a branch, false if it is a leaf.
     * (overwritten in MenuSub etc.)
     */
    virtual bool isBranch() {
	return false;
    };

    /**
     * Removes all child entries from the menu node (gui) and
     * deletes the MenuNode objects (memory).
     */
    virtual void clear();

    /** returns a pointer to the menu's parent node */
    virtual MenuNode *getParentNode();

    /** Returns the number of ids a menu needs */
    virtual int getNeededIDs();

    /**
     * Returns true if the node is enabled.
     */
    virtual bool isEnabled();

    /**
     * Enables/disables the current menu node.
     * @param enable true to enable the item, false to disable
     */
    virtual void setEnabled(bool enable);

    /** Returns true if the node is checked. */
    virtual bool isChecked();

    /**
     * Sets or removes the checkmark from a menu node.
     * @param id the item's menu id
     * @param check true to set the mark, false to remove
     */
    virtual void setItemChecked(int item, bool check);

    /**
     * Sets/removes the checkmark from the current menu node.
     * @param check true to set the mark, false to remove
     */
    virtual void setChecked(bool check);

    /**
     * Sets the visible text of an item to a new value. (Only useful
     * for a MenuItem)
     * @param text the new text
     */
    virtual void setText(const QString &text);

    /**
     * Tries to find a menu node by it's unique id string. It descends
     * recursively through all child nodes if necessary.
     * @param uid the unique id string to be searched
     * @return pointer to the found node or 0 if not found
     */
    MenuNode *findUID(const QString &uid);

    /**
     * Tries to find a child node by it's name.
     * @param name non-localized name of the child node
     * @return pointer to the found node or 0 if not found
     */
    MenuNode *findChild(const QString &name);

    /**
     * Tries to find a child node by it's unique id.
     * @param id menu id of the child node
     * @return pointer to the found node or 0 if not found
     */
    MenuNode *findChild(int id);

    /**
     * Removes a child node of the curren node. If the child
     * was not found or is already removed this does nothing.
     * @param child pointer to the child node
     */
    virtual void removeChild(MenuNode *child);

    /**
     * Inserts a new branch node into the menu structure. The new node
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
    virtual MenuNode *insertLeaf(const QString &name,
	const QString &command, int key, const QString &uid,
	int index = -1);

    /**
     * Registers a node as a child of the current node.
     * @param node pointer to the child node
     * @return the id of the node
     */
    virtual int registerChild(MenuNode *node);

    /**
     * Inserts a new child node into the structure. If the specified
     * position contains a path that doesn't completely exist, all
     * missing branches will be appended.
     * @param name non-localized name of the first node (might be 0)
     * @param position path consiting of several node names separated
     *        by a '/'. All strings are non-localized.
     * @param command the command to be sent when the node is
     *                selected (might be 0)
     * @param key bitmask of the keyboard shortcut (see "qkeycode.h"),
     *            0 if unused
     * @param uid unique id string (might be 0)
     */
    virtual int insertNode(const QString &name, const QString &position,
	const QString &command, int key, const QString &uid);

    /**
     * Converts a child node from leaf to branch type by removing
     * the leaf and inserting a branch with the same properties
     * instead.
     * @param node the child node to be converted
     * @return pointer to the new branch node
     */
    virtual MenuNode *leafToBranch(MenuNode *node);

    /**
     * Handles/interpretes special menu commands.
     * @param command name of a menu node or command
     * @return true if the name was recognized as a command and handled
     */
    virtual bool specialCommand(const QString &command);

    /**
     * Called to notify the node that it has been selected.
     */
    virtual void actionSelected();

    /**
     * Informs the node that the enabled state of a child node
     * might have changed.
     * @param id menu id of the child node
     * @param enable true if the item has been enabled, false if disabled
     */
    virtual void actionChildEnableChanged(int id, bool enable);

    /**
     * Returns a reference to the list of groups. It recursively calls
     * all parent node's getGroupList() function until it reaches the
     * root node of the menu structure that holds the list of groups and
     * overwrites this function.
     * @return reference to the list of groups
     */
    virtual QDict<MenuNode> *getGroupList();

    /**
     * Adds the node to a group. If it is already a member of the
     * group this function will do nothing.
     * @param group name of the group
     */
    void joinGroup(const QString &group);

    /**
     * Removes the node from a group (opposite of joinGroup).
     * @param group name of the group
     */
    void leaveGroup(const QString &group);

protected:

    /**
     * Returns the address of the root node of the menu structure.
     */
    MenuNode *getRootNode();

    /**
     * Emits a command if the node is the root node. If it is a client
     * node it will call the root node's emitCommand() function.
     * @see #emitCommand()
     * @see #getRootNode()
     */
    void emitCommand(const QString &command);

signals:
    /**
     * Will be emitted if the command of the menu node
     * should be executed. It will only be emitted by the
     * root node, client nodes call the root node's emitCommand()
     * function.
     * @see #emitCommand()
     * @see #getRootNode()
     */
    void sigCommand(const QString &command);

    /**
     * Parent nodes can connect to this signal in order to get notified
     * when the enable state of their child node has changed.
     * @param id menu id of the child node
     * @param enable true if the item has been enabled, false if disabled
     */
    void sigChildEnableChanged(int id, bool enable);

    /**
     * Client nodes can be connected to this signal in order to get
     * notified if the enable state of their parent has changed.
     */
    void sigParentEnableChanged();

private slots:
    /**
     * Informs the node that the enabled state of a child node
     * might have changed.
     * @param id menu id of the child node
     * @param enable true if the item has been enabled, false if disabled
     */
    void slotChildEnableChanged(int id, bool enable);

    /**
     * Informs the node that the enabled state of it's parent might
     * have changed.
     * @param enable true if the item has been enabled, false if disabled
     */
    void slotParentEnableChanged();

protected:

    /** list with pointers to child menus */
    QList<MenuNode> m_children;

    /** list of group names the item belongs to */
    QStringList m_groups;

private:
    /** numeric id in the menu */
    int m_id;

    /** unique id string */
    QString m_uid;

    /** bitmask of the keyboard shortcut */
    int m_key;

    /** name of the node (non-localized) */
    QString m_name;

    /** command to be sent when the node is activated (optional) */
    QString m_command;

    /** icon of the node (optional) */
    QPixmap m_icon;

    /** parent of this entry */
    MenuNode* m_parentNode;

    /** true if the item is enabled (default=true) */
    bool m_enabled;

    /** last value of the enabled flag (for detecting changes) */
    bool m_last_enabled;

    /** true if the item is checked (default=false) */
    bool m_checked;
};

#endif // _MENU_NODE_H_
