/***************************************************************************
			  MenuSub.cpp  -  description
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

#include "config.h"
#include <stdio.h>
#include <qapplication.h> // only needed for an ugly workaround :-[
#include <klocale.h>

#include "MenuItem.h"
#include "MenuSub.h"

//***************************************************************************
MenuSub::MenuSub(MenuNode *parent, const QString &name,
	const QString &command, int key, const QString &uid)
    :MenuItem(parent, name, command, key, uid),
    m_menu(0, i18n(name))
{
    QObject::connect(&m_menu, SIGNAL(activated(int)),
		     this, SLOT(slotSelected(int)));
}

//***************************************************************************
MenuSub::~MenuSub()
{
}

//***************************************************************************
int MenuSub::getChildIndex(int id)
{
    return m_menu.indexOf(id);
}

//***************************************************************************
QPopupMenu &MenuSub::getPopupMenu()
{
    return m_menu;
}

//***************************************************************************
MenuNode *MenuSub::insertBranch(const QString &name, const QString &command,
                                int key, const QString &uid, int /*index*/)
{
    MenuSub *node = new MenuSub(this, name, command, key, uid);
    ASSERT(node);
    if (!node) return 0;

    int new_id = registerChild(node);
    m_menu.insertItem(i18n(node->getName()),
		      &(node->getPopupMenu()), new_id);

    return node;
}

//***************************************************************************
MenuNode *MenuSub::insertLeaf(const QString &name, const QString &command,
                              int key, const QString &uid, int /*index*/) {
    int new_id;
    ASSERT(name.length());
    if ( !name.length() ) return 0;

    MenuItem *item = new MenuItem(this, name, command, key, uid);
    ASSERT(item);
    if (!item) return 0;

    new_id = registerChild(item);
    m_menu.insertItem(i18n(name), new_id);
    m_menu.setAccel(key, new_id);

    return item;
}

//***************************************************************************
void MenuSub::removeChild(MenuNode *child)
{
    ASSERT(child);
    if (!child) return;
    if (m_children.findRef(child) == -1) return;

    m_menu.removeItem(child->getId());
    MenuItem::removeChild(child);
}

//***************************************************************************
bool MenuSub::specialCommand(const QString &command)
{
    ASSERT(command.length());
    if (!command.length()) return false;

    if (command.startsWith("#exclusive")) {
	// debug("MenuSub(%s) >> exclusive <<", getName());
	return true;
    } else if (command.startsWith("#number")) {
	// debug("MenuSub(%s) >> number <<", getName());
	return true;
    } else if (command.startsWith("#separator")) {
	m_menu.insertSeparator( -1);
	return true;
    }

    return MenuItem::specialCommand(command);
}

//***************************************************************************
void MenuSub::actionChildEnableChanged(int id, bool enable)
{
    MenuNode::actionChildEnableChanged(id, enable);
    m_menu.setItemEnabled(id, enable);

    /**
     * @todo remove this workaround as soon as Qt3 is sane again.
     * Without this ugly call, if a menu entry belongs to a sub menu, it
     * sometimes is not correctly re-enabled if it's parent menu has been
     * disabled and re-enabled.
     */
    qApp->processOneEvent();
}

//***************************************************************************
void MenuSub::slotSelected(int id)
{
    MenuNode *child = findChild(id);
    if (child) {
	child->actionSelected();
    } else {
	warning("MenuSub::slotSelected: child with id #%d not found!", id);
    }
}

//***************************************************************************
void MenuSub::setItemIcon(int id, const QPixmap &icon)
{
    m_menu.changeItem(icon, m_menu.text(id), id);
}

//***************************************************************************
void MenuSub::setItemChecked(int id, bool enable)
{
    if ( !m_menu.findItem(id) ) return ;
    m_menu.setItemChecked(id, enable);
}

/* end of libgui/MenuSub.cpp */
