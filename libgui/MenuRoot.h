/***************************************************************************
			  MenuRoot.h  -  root node of a menu structure
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
#define _MENU_ROOT_H_

#include "config.h"

#include <QList>

#include "MenuNode.h"

class KMenuBar;

namespace Kwave
{

    class MenuSub;

    /**
     * This is the class for the root of a Menu (e.g. a MenuBar) that contains
     * all toplevel menues of a menu hierarchy.
     * @author Thomas Eschenbacher
     */
    class MenuRoot : virtual public Kwave::MenuNode
    {
	Q_OBJECT

    public:

	/**
	 * Constructor.
	 * @param bar reference to a KMenuBar
	 */
	MenuRoot(KMenuBar &bar);

	/** Destructor */
	virtual ~MenuRoot();

	/**
	 * overloaded version from MenuNode, which does a cleanup of
	 * the "garbage collector" afterwards.
	 *
	 * @see MenuNode::insertNode()
	 *
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
	 * Removes a child node of the curren node. If the child
	 * was not found or is already removed this does nothing.
	 * @param child pointer to the child node
	 */
	virtual void removeChild(Kwave::MenuNode *child);

	/**
	 * Handles/interpretes special menu commands.
	 * @param command name of a menu node or command
	 * @return true if the name was recognized as a command and handled
	 */
	virtual bool specialCommand(const QString &command);

	/**
	 * Returns a pointer to the list of groups
	 */
	virtual QHash<QString, Kwave::MenuGroup *> &getGroupList();

	/**
	 * replacement for QObject::deleteLater(...), which does not work
	 * in this context
	 */
	static void deleteLater(Kwave::MenuNode *node);

    private:

	/** reference to a KMenuBar */
	KMenuBar &m_menu_bar;

	/** list of menu groups */
	QHash<QString, Kwave::MenuGroup *> m_group_list;

	/** list of nodes to delete, as deleteLater() does not work :-( */
	static QList <Kwave::MenuNode *> m_garbage;

    };
}

#endif // _MENU_ROOT_H_

//***************************************************************************
//***************************************************************************
