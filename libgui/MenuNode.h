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

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtGui/QIcon>
#include <QtGui/QKeySequence>
#include <QtGui/QPixmap>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "libgui/MenuGroup.h"

class QAction;

namespace Kwave
{

    class MenuSub;

    /**
     * Base class for the MenuRoot, MenuEntry, SubMenu and
     * the ToplevelMenu class.
     */
    class MenuNode: public QObject
    {
	Q_OBJECT

    public:

	/**
	 * Constructor.
	 * @param parent pointer to the node's parent (might be 0)
	 * @param name the non-localized name of the node
	 * @param command the command to be sent when the node is
	 *                selected (optional, default=0)
	 * @param shortcut keyboard shortcut (optional, default=0)
	 * @param uid unique id string (optional, default=0)
	 */
	MenuNode(Kwave::MenuNode *parent,
	         const QString &name,
	         const QString &command,
	         const QKeySequence &shortcut,
	         const QString &uid);

	/**
	 * Destructor. Clears the menu node and cleans up.
	 * @see #clear()
	 */
	virtual ~MenuNode();

	/** Returns the (non-localized) name of the node. */
	inline const QString &name() const { return m_name; }

	/** Returns the command of the node. */
	inline const QString &command() const { return m_command; }

	/** Returns the corresponding menu action */
	virtual QAction *action() { return 0; }

	/**
	 * Returns the unique id string of the node.
	 */
	inline const QString &uid() const { return m_uid; }

	/**
	 * Sets the unique id string of the node
	 */
	void setUID(const QString &uid);

	/**
	 * Returns the bitmask of the keyboard shortcut.
	 */
	inline int shortcut() const { return m_shortcut; }

	/**
	 * Sets the bitmask of the keyboard shortcut.
	 */
	virtual void setShortcut(const QKeySequence &shortcut) {
	    m_shortcut = shortcut;
	}

	/**
	 * Returns the menu nodes' icon.
	 */
	virtual const QIcon icon();

	/**
	 * Sets a new icon of a menu node.
	 * @param icon QIcon with the icon
	 */
	virtual void setIcon(const QIcon &icon);

	/**
	 * Returns true if the node is a branch, false if it is a leaf.
	 * (overwritten in MenuSub etc.)
	 */
	virtual bool isBranch() const { return false; }

	/**
	 * Removes all child entries from the menu node (gui) and
	 * deletes the MenuNode objects (memory).
	 */
	virtual void clear();

	/** returns a pointer to the menu's parent node */
	virtual Kwave::MenuNode *parentNode() const;

	/**
	 * Returns true if the node is enabled.
	 */
	virtual bool isEnabled();

	/**
	 * Enables/disables the current menu node.
	 * @param enable true to enable the item, false to disable
	 */
	virtual void setEnabled(bool enable);

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
	Kwave::MenuNode *findUID(const QString &uid);

	/**
	 * Tries to find a child node by it's name.
	 * @param name non-localized name of the child node
	 * @return pointer to the found node or 0 if not found
	 */
	Kwave::MenuNode *findChild(const QString &name);

	/**
	 * Removes a child node of the curren node. If the child
	 * was not found or is already removed this does nothing.
	 * @param child pointer to the child node
	 */
	virtual void removeChild(Kwave::MenuNode *child);

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
	 * @param shortcut keyboard shortcut, 0 if unused
	 * @param uid unique id string (might be 0)
	 * @return pointer to the new branch node
	 */
	virtual Kwave::MenuSub *insertBranch(const QString &name,
	                                     const QString &command,
	                                     const QKeySequence &shortcut,
	                                     const QString &uid);

	/**
	 * Inserts a new leaf node into the menu structure. The new node
	 * normally is (derived from) MenuItem.
	 * @param name non-localized name of the node
	 * @param command the command to be sent when the node is
	 *                selected (might be 0)
	 * @param shortcut keyboard shortcut, 0 if unused
	 * @param uid unique id string (might be 0)
	 * @return pointer to the new leaf node
	 */
	virtual Kwave::MenuNode *insertLeaf(const QString &name,
	                                    const QString &command,
	                                    const QKeySequence &shortcut,
	                                    const QString &uid);

	/**
	 * Registers a node as a child of the current node.
	 * @param node pointer to the child node
	 */
	virtual void registerChild(Kwave::MenuNode *node);

	/**
	 * Inserts a new child node into the structure. If the specified
	 * position contains a path that doesn't completely exist, all
	 * missing branches will be appended.
	 * @param name non-localized name of the first node (might be 0)
	 * @param position path consiting of several node names separated
	 *        by a '/'. All strings are non-localized.
	 * @param command the command to be sent when the node is
	 *                selected (might be 0)
	 * @param shortcut keyboard shortcut, 0 if unused
	 * @param uid unique id string (might be 0)
	 */
	virtual void insertNode(const QString &name,
	                        const QString &position,
	                        const QString &command,
	                        const QKeySequence &shortcut,
	                        const QString &uid);

	/**
	 * Converts a child node from leaf to branch type by removing
	 * the leaf and inserting a branch with the same properties
	 * instead.
	 * @param node the child node to be converted
	 * @return pointer to the new branch node
	 */
	virtual Kwave::MenuNode *leafToBranch(Kwave::MenuNode *node);

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
	 * Returns a reference to the list of groups. It recursively calls
	 * all parent node's groupList() function until it reaches the
	 * root node of the menu structure that holds the list of groups and
	 * overwrites this function.
	 * @return reference to the list of groups
	 */
	virtual QHash<QString, Kwave::MenuGroup *> &groupList();

	/**
	 * Adds the node to a group. If it is already a member of the
	 * group this function will do nothing.
	 * @param group name of the group
	 * @param mode the mode of the group (normal or exclusive)
	 */
	void joinGroup(const QString &group, Kwave::MenuGroup::Mode mode);

	/**
	 * Removes the node from a group (opposite of joinGroup).
	 * @param group name of the group
	 */
	void leaveGroup(const QString &group);

    protected:

	/**
	 * Returns the address of the root node of the menu structure.
	 */
	Kwave::MenuNode *getRootNode();

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

    protected:

	/** list with pointers to child menus */
	QList<Kwave::MenuNode *> m_children;

	/** list of group names the item belongs to */
	QStringList m_groups;

    private:

	/** unique id string */
	QString m_uid;

	/** bitmask of the keyboard shortcut */
	QKeySequence m_shortcut;

	/** name of the node (non-localized) */
	QString m_name;

	/** command to be sent when the node is activated (optional) */
	QString m_command;

	/** parent of this entry */
	Kwave::MenuNode *m_parentNode;

    };
}

#endif // _MENU_NODE_H_

//***************************************************************************
//***************************************************************************
