/***************************************************************************
                          MenuRoot.cpp  -  description
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

#include <kapp.h>
#include <kmenubar.h>

#include "MenuItem.h"
#include "MenuToplevel.h"
#include "MenuRoot.h"

//*****************************************************************************
MenuRoot::MenuRoot(KMenuBar &bar)
    :MenuNode(this, "(root)"), menu_bar(bar)
{
}

//*****************************************************************************
int MenuRoot::getChildIndex(int id)
{
    for (unsigned int i=0; i < menu_bar.count(); i++) {
	if (menu_bar.idAt(i) == id) return i;
    }
    return -1;
}

//*****************************************************************************
MenuNode *MenuRoot::insertBranch(char *name, int key,
                                 char *uid, int index)
{
    MenuToplevel *node = new MenuToplevel(this, name, 0, key, uid);
    int new_id = registerChild(node);
    menu_bar.insertItem(klocale->translate(name),
	node->getPopupMenu(), new_id, index);
    return node;
}

//*****************************************************************************
MenuNode *MenuRoot::insertLeaf(char *name, char *command,
                               int key, char *uid,
                               int index)
{
    MenuItem *item = new MenuItem(this, name, command, key, uid);
    int new_id = registerChild(item);
    menu_bar.insertItem(klocale->translate(name),
	new_id, index);
    return item;
}

//*****************************************************************************
void MenuRoot::removeChild(int id)
{
    MenuNode::removeChild(id);
    menu_bar.removeItem(id);
}

//*****************************************************************************
bool MenuRoot::setItemEnabled(int id, bool enable)
{
    if (!findChild(id)) return false;
    menu_bar.setItemEnabled(id, enable);
    return true;
}

/* end of MenuRoot.cpp */
