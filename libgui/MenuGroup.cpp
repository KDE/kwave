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

    // notification for the childs that our enable state changed
    QObject::disconnect(
	this, SIGNAL(sigParentEnableChanged()),
	child, SLOT(slotParentEnableChanged())    	
    );

    int index = children.find(child);
    if (index != -1) {
	children.remove(index);
    }
}

//*****************************************************************************
void MenuGroup::setEnabled(bool enable)
{
    debug("MenuGroup(%s)::setEnabled(%d)", getName(), enable);
    // MenuNode::setEnabled(enable);
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
    debug("MenuGroup(%s)::selectItem(%s)", getName(), uid);

    MenuNode *new_selection = 0;
    MenuNode *child = children.first();
    while (child) {
	int pos = children.at();
	if (strcmp(child->getUID(), uid) == 0)
	    new_selection = child; // new selected child found !
	else
	    child->setChecked(false); // remove check from others
	
	children.at(pos);
	child = children.next();
    }

    // select the new one if found
    if (new_selection) new_selection->setChecked(true);

}

/* end of libgui/MenuGroup.cpp */
