/***************************************************************************
			  MenuGroup.cpp  -  description
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
#include <kapp.h>

#include "MenuNode.h"
#include "MenuGroup.h"

//*****************************************************************************
MenuGroup::MenuGroup(MenuNode *parent, char *name)
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

    children.append(child);

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

    int index = children.find(child);
    if (index != -1) {
//	debug("MenuGroup::removeChild(%s)",child->getName());
	// notification for the childs that our enable state changed
	QObject::disconnect(
	    this, SIGNAL(sigParentEnableChanged()),
	    child, SLOT(slotParentEnableChanged())
	);

	children.setAutoDelete(false);
	children.remove(child);

	child->leaveGroup(getName());
    }
}

//*****************************************************************************
void MenuGroup::setEnabled(bool enable)
{
    MenuNode *child = children.first();
    while (child) {
	int pos = children.at();
	child->setEnabled(enable);
	children.at(pos);
	child = children.next();
    }
}

//*****************************************************************************
void MenuGroup::selectItem(const char *uid)
{
    MenuNode *new_selection = 0;
    MenuNode *child = children.first();
    while (child) {
	int pos = children.at();
	if (strcmp(child->getUID(), uid) == 0)
	    new_selection = child;    // new selected child found !
	else
	    child->setChecked(false);    // remove check from others

	children.at(pos);
	child = children.next();
    }

    // select the new one if found
    if (new_selection) new_selection->setChecked(true);

}

//*****************************************************************************
void MenuGroup::clear()
{
    // deregister all child nodes from us
    MenuNode *child = children.first();
    while (child) {
	removeChild(child);
	child = children.first();
    }
}

/* end of libgui/MenuGroup.cpp */
