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

#include <stdio.h>
#include <qpopupmenu.h>
#include <kapp.h>

#include "MenuItem.h"
#include "MenuSub.h"

MenuSub::MenuSub(MenuNode *parent, char *name, char *command,
                 int key, char *uid)
    :MenuNode(parent, name, command, key, uid)
{
    menu = new QPopupMenu(0, klocale->translate(name));

    QObject::connect(menu,SIGNAL(activated(int)),
	this,SLOT(slotSelected(int)));
}

int MenuSub::getChildIndex(int id)
{
    return (menu) ? menu->indexOf(id) : -1;
}

QPopupMenu *MenuSub::getPopupMenu()
{
    return menu;
}

MenuNode *MenuSub::insertBranch(char *name, int key, char *uid, int index)
{
    MenuSub *node = new MenuSub(this, name, 0, key, uid);

    if (menu) {
	int new_id = registerChild(node);
	menu->insertItem(klocale->translate(node->getName()),
	    node->getPopupMenu(), new_id);
    }

//    debug("MenuSub(%s): insertBranch(%s)", getName(), name);
    return node;
}

MenuNode *MenuSub::insertLeaf(char *name, char *command, int key,
                              char *uid, int index=-1)
{
    int new_id;
    if ((!name) || (!menu)) return 0;

    MenuItem *item = new MenuItem(this, name, command, key, uid);
    if (!item) return 0;

    new_id = registerChild(item);
    menu->insertItem(klocale->translate(name), new_id);
    menu->setAccel(key, new_id);

//    debug("MenuSub(%s): insertLeaf(%s)", getName(), name);
    return item;
}

void MenuSub::removeChild(int id)
{
    MenuNode::removeChild(id);
    menu->removeItem(id);
}

bool MenuSub::specialCommand(const char *command)
{
    if (strcmp(command, "listmenu") == 0) {
	// insert a list menu
	return true;
    } else if (strcmp(command, "exclusive") == 0) {
	//
	return true;
    } else if (strcmp(command, "number") == 0) {
	//
	return true;
    } else if (strcmp(command, "separator") == 0) {
	menu->insertSeparator(-1);
    	return true;
    } else if (strcmp(command, "checkable") == 0) {
	return true;
    }

    return false;
}

void MenuSub::slotSelected(int id)
{
    MenuNode *child = findChild(id);
    if (child) {
	child->actionSelected();
    } else {
	debug("MenuSub::slotSelected: child with id #%d not found!", id);
    }
}
