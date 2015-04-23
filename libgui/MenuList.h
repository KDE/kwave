/***************************************************************************
             MenuList.h  -  placeholder for a list of menu entries
			     -------------------
    begin                : Wed Dec 03 2014
    copyright            : (C) 2014 by Thomas Eschenbacher
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

#ifndef MENU_LIST_H
#define MENU_LIST_H

#include "config.h"

#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtGui/QMenu>
#include <QtCore/QString>

#include "libgui/MenuNode.h"

namespace Kwave
{
    /**
     * Container for a group of entries that belong to a list of menu entries,
     * which contains a number of list entries. It is normally owned by a
     * sub menu.
     */
    class MenuList : public Kwave::MenuNode
    {
	Q_OBJECT

    public:
	/**
	 * Constructor.
	 * @param parent pointer to the node's parent (might be 0)
	 * @param command the command to be sent when the entry is
	 *                selected, should contain a wildcard (%1)
	 * @param uid unique id string (optional, default=0)
	 */
	MenuList(Kwave::MenuNode *parent,
	         const QString &command,
	         const QString &uid);

	/** Destructor */
	virtual ~MenuList();

	/**
	 * Always returns true, as the nodes of this type are no branches.
	 */
	virtual bool isBranch() const { return false; }

	/**
	 * Removes all entries from the list menu.
	 */
	virtual void clear();

	/**
	 * Inserts a new leaf node under the corresponding submenu.
	 * The new node normally is (derived from) MenuItem.
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

    };
}

#endif /* MENU_LIST_H */

//***************************************************************************
//***************************************************************************

