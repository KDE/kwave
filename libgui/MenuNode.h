/***************************************************************************
                          MenuNode.h  -  description
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

#ifndef MENUNODE_H
#define MENUNODE_H

#include <qobject.h>
#include <qlist.h>

/**
 * Base class for the MenuRoot, MenuEntry, SubMenu and
 * the ToplevelMenu class.
 * @author Thomas Eschenbacher
 */
class MenuNode: public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor.
     * @param pointer to the node's parent (might be 0)
     * @param name the non-localized name of the node
     * @param command the command to be sent when the node is
     *                selected (optional, default=0)
     */
    MenuNode(MenuNode *parent, const char *name, const char *command = 0);

    /**
     * Destructor. Clears the menu node and cleans up.
     * @see #clear()
     */
    virtual ~MenuNode();

    /**
     * Returns the (non-localized) name of the node.
     */
    inline const char   *getName() const {return name;};

    /**
     * Returns the unique if of the node.
     */
    inline int           getId()       {return this->id;};

    /**
     * Sets the unique id of a node.
     * @param id new unique id of the node
     */
    inline void          setId(int id) {this->id=id;};

    /**
     * Positional index of the node in the parent node.
     * (overwritten in MenuItem and MenuSub)
     * @return index [0..n] or -1 if no parent
     */
    virtual int getIndex() {return -1;};

    /**
     * Returns the positional index of a child node, identified by
     * it's unique id.
     * @param unique id of the child
     * @return index [0..n] or -1 f not found
     */
    virtual int getChildIndex(const int id);

    /**
     * Returns true if the node is a branch, false if it is a leaf.
     * (overwritten in MenuSub etc.)
     */
    virtual bool isBranch() {return false;};

/* ###
         void          setEnabled  (bool enable);
         void          checkEntry  (int id);
         void          checkEntry  (int id, const bool check);
         int           getCheckedId() { return checked; };
         void          setCheckedId(int id) { this->checked = id;};
         void          setCheckable ();
         void          setNumberable();
### */

    /**
     * Removes all child entries from the menu node (gui) and
     * deletes the MenuNode objects (memory).
     */
    virtual void clear();

    /**
     * Sets a new pointer to the node's parent.
     * @param newParent pointer to a MenuNode,
     */
    virtual void setParent(MenuNode *newParent);

    /** returns a pointer to the menu's parent node */
    virtual MenuNode *getParentNode();

    /** Returns the number of ids a menu needs */
    virtual int getNeededIDs();

    /**
     * Enables or disables a menu node. If the specified item is not
     * a member of the current menu, this method will recursively call all
     * of it's child nodes.
     * @param id the unique menu item id
     * @param enable true to enable the item, false to disable
     * @return true if the item has been found, false if not
     */
    virtual bool setItemEnabled(const int item, const bool enable);

    /**
     * Tries to find a child node by it's name.
     * @param name non-localized name of the child node
     * @return pointer to the found node or 0 if not found
     */
    MenuNode *findChild(const char *name);

    /**
     * Tries to find a child node by it's unique id.
     * @param id unique id of the child node
     * @return pointer to the found node or 0 if not found
     */
    MenuNode *findChild(const int id);

    /**
     * Removes a child, identified by it's unique id. If the child
     * was not found or is already removed this does nothing.
     * @param id unique id
     */
    virtual void removeChild(const int id);

    /**
     * Inserts a new branch node into the menu structure. The new node
     * normally is (derived from) MenuSub.
     * @param name non-localized name of the node
     * @param key bitmask with the keyboard shortcut (see "qkeycode.h"),
     *            0 if unused
     * @param uid unique id, must be [0...]
     * @param index the positional index within the parent menu, starting
     *              from 0 or -1 for appending. Optional, default=-1
     * @return pointer to the new branch node
     */
    virtual MenuNode *insertBranch(char *name, const int key,
                                   const char *uid, const int index=-1);

    /**
     * Inserts a new leaf node into the menu structure. The new node
     * normally is (derived from) MenuItem.
     * @param name non-localized name of the node
     * @param command the command to be sent when the node is
     *                selected (might be 0)
     * @param key bitmask with the keyboard shortcut (see "qkeycode.h"),
     *            0 if unused
     * @param uid unique id, must be [0...]
     * @param index the positional index within the parent menu, starting
     *              from 0 or -1 for appending. Optional, default=-1
     * @return pointer to the new leaf node
     */
    virtual MenuNode *insertLeaf(char *name, const char *command,
                                 const int key, const char *uid,
                                 const int index=-1);

    /**
     * Registers a node as a child of the current node.
     * @param node pointer to the child node
     * @return the unique id of the node
     */
    virtual int registerChild(MenuNode *node);

    /**
     * Inserts a new child node into the structure. If the specified
     * position contains a path that doesn't completely exist, all
     * missing branches will be appended.
     * @param name non-localized name of the first node (might be 0)
     * @param position path consiting of several node names separated
     *        by a '/'. All strings are non-localized.
     * @param key bitmask with the keyboard shortcut (see "qkeycode.h"),
     *            0 if unused
     * @param uid unique id, must be [0...]
     */
    virtual int insertNode(const char *command, char *name, char *position,
	                   const int key, const char *uid);

    /**
     * Handles/interpretes special menu commands.
     * @param command name of a menu node or command
     * @return true if the name was recognized as a command and handled
     */
    virtual bool specialCommand(const char *command);

    /**
     * Called to notify the node that it has been selected.
     */
    virtual void actionSelected();

private:

    /** list with pointers to child menus */
    QList<MenuNode> children;

    /** unique id of the node */
    int   id;

    /** name of the node (non-localized) */
    char *name;

    /** command to be sent when the node is activated (optional) */
    char *command;

    /** parent of this entry */
    MenuNode* parentNode;
};

#endif
