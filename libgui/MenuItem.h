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

#ifndef MENU_ITEM_H
#define MENU_ITEM_H

#include "config.h"

#include <QAction>
#include <QString>

#include "libgui/MenuNode.h"

namespace Kwave
{

    /**
     * Base class for entries in a Menu. It is normally owned by a toplevel
     * menu or a submenu.
     */
    class MenuItem: public Kwave::MenuNode
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
	MenuItem(Kwave::MenuNode *parent,
	         const QString &name,
	         const QString &command,
	         const QKeySequence &shortcut,
	         const QString &uid);

	/** virtual destructor */
	virtual ~MenuItem();

	/**
	 * Called to notify the item that it has been selected.
	 */
	void actionSelected() Q_DECL_OVERRIDE;

	/**
	 * Handles/interpretes special menu commands.
	 * @param command name of a menu node or command
	 * @return true if the name was recognized as a command and handled
	 */
	bool specialCommand(const QString &command) Q_DECL_OVERRIDE;

	/**
	 * Shows/hides the current menu node.
	 * @param visible true to show the item, false to hide
	 */
	void setVisible(bool visible) Q_DECL_OVERRIDE;

	/**
	 * Returns true if the node is enabled.
	 */
	bool isEnabled() Q_DECL_OVERRIDE;

	/**
	 * Enables/disables the current menu node.
	 * @param enable true to enable the item, false to disable
	 */
	void setEnabled(bool enable) Q_DECL_OVERRIDE;

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
	void setChecked(bool check) Q_DECL_OVERRIDE;

	/**
	 * Sets the visible text of an item to a new value. (Only useful
	 * for a MenuItem)
	 * @param text the new text
	 */
	void setText(const QString &text) Q_DECL_OVERRIDE;

	/**
	 * Returns the menu nodes' icon.
	 */
	const QIcon icon() Q_DECL_OVERRIDE;

	/**
	 * Sets a new icon of a menu node.
	 * @param icon QPixmap with the icon
	 */
	void setIcon(const QIcon &icon) Q_DECL_OVERRIDE;

	/** Returns the corresponding menu action */
	QAction *action() Q_DECL_OVERRIDE { return &m_action; }

    private slots:

	virtual void actionTriggered(bool checked);

    private:

	/**
	 * name of a group for exclusive selection
	 * (optional, set by special command, default=0)
	 */
	QString m_exclusive_group;

	/** the QAction behind this menu entry */
	QAction m_action;

    };
}

#endif // _MENU_ITEM_H_

//***************************************************************************
//***************************************************************************
