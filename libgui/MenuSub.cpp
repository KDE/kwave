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

MenuSub::MenuSub (const char *name)
    :MenuNode(name)
{
    menu = new QPopupMenu(0, klocale->translate(name));
}

int MenuSub::getChildIndex(const int id)
{
    return (menu) ? menu->indexOf(id) : -1;
}

QPopupMenu *MenuSub::getPopupMenu()
{
    return menu;
}

MenuNode *MenuSub::insertBranch(char *name, const int key, const char *uid,
				const int index)
{
    MenuSub *node = new MenuSub(name);

    int new_id = registerChild(node);
    menu->insertItem(klocale->translate(node->getName()),
	node->getPopupMenu(), new_id);

//    debug("MenuSub(%s): insertBranch(%s)", getName(), name);
    return node;
}

MenuNode *MenuSub::insertLeaf(const char *command, char *name,
                              const int key, const char *uid,
                              const int index=-1)
{
    int new_id;
    if ((!name) || (!menu)) return 0;

    MenuItem *item = new MenuItem(name);
    if (!item) return 0;

    new_id = registerChild(item);
    menu->insertItem(klocale->translate(name), new_id);
    menu->setAccel(key, new_id);

//    debug("MenuSub(%s): insertLeaf(%s)", getName(), name);
    return item;
}

void MenuSub::removeChild(const int id)
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
