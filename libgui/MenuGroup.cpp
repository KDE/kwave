/***************************************************************************
	  MenuGroup.cpp  - controls a group of menu nodes
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
#include <qlist.h>
#include <kapp.h>

#include "MenuNode.h"
#include "MenuGroup.h"

//*****************************************************************************
MenuGroup::MenuGroup(MenuNode *parent, const QString &name)
    :MenuNode(parent, name, 0, 0, name)
{
    if (parent) parent->registerChild(this);
}

//*****************************************************************************
MenuGroup::~MenuGroup()
{
    clear();

    QDict<MenuNode> *group_list = getGroupList();

    if (group_list && (group_list->find(getName()) != 0)) {
	group_list->remove(getName());
    }
}

//*****************************************************************************
int MenuGroup::registerChild(MenuNode *child)
{
    if (!child) return -1;

    m_children.append(child);

    // notification for the childs that our enable state changed
    QObject::connect(
	this, SIGNAL(sigParentEnableChanged()),
	child, SLOT(slotParentEnableChanged())
    );

    return child->getId();
}

//*****************************************************************************
void MenuGroup::removeChild(MenuNode *child)
{
    if (!child) return;
    if (!m_children.contains(child)) return;

    // notification for the childs that our enable state changed
    QObject::disconnect(
	this, SIGNAL(sigParentEnableChanged()),
	child, SLOT(slotParentEnableChanged())
    );

    m_children.setAutoDelete(false);
    m_children.remove(child);

    child->leaveGroup(getName());
}

//*****************************************************************************
void MenuGroup::setEnabled(bool enable)
{
    QListIterator<MenuNode> it(m_children);
    while (it.current()) {
	it.current()->setEnabled(enable);
	++it;
    }
}

//*****************************************************************************
void MenuGroup::selectItem(const QString &uid)
{
    MenuNode *new_selection = 0;

    QListIterator<MenuNode> it(m_children);
    for ( ; it.current(); ++it) {
	MenuNode *child = it.current();
	if (uid == child->getUID())
	    new_selection = child;    // new selected child found !
	else
	    child->setChecked(false);    // remove check from others
    }

    // select the new one if found
    if (new_selection) new_selection->setChecked(true);

}

//*****************************************************************************
void MenuGroup::clear()
{
    // deregister all child nodes from us
    MenuNode *child = m_children.first();
    while (child) {
	removeChild(child);
	child = m_children.first();
    }
}

/* end of libgui/MenuGroup.cpp */
