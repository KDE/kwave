/***************************************************************************
                          MenuRoot.cpp  -  root node of a menu structure
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
    :MenuNode(0, "(root)"), menu_bar(bar)
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
MenuNode *MenuRoot::insertBranch(char *name, char *command, int key,
                                 char *uid, int index)
{
    MenuToplevel *node = new MenuToplevel(this, name, command, key, uid);
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
void MenuRoot::removeChild(MenuNode *child)
{
    if (!child) return;
    MenuNode::removeChild(child);
    menu_bar.removeItem(child->getId());
}

//*****************************************************************************
void MenuRoot::actionChildEnableChanged(int id, bool enable)
{
    // do nothing, the child nodes of the toplevel menu have already
    // been enabled/disabled
    // we don't want to -> "menu_bar.setItemEnabled(id, enable);" !!!
}

/* end of MenuRoot.cpp */
