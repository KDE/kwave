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

/**
 * Constructor.
 * @param bar reference to a KMenuBar
 */
MenuRoot::MenuRoot(KMenuBar &bar)
    :MenuNode(this, "(root)"), menu_bar(bar)
{
}

int MenuRoot::getChildIndex(const int id)
{
    for (unsigned int i=0; i < menu_bar.count(); i++) {
	if (menu_bar.idAt(i) == id) return i;
    }
    return -1;
}

MenuNode *MenuRoot::insertBranch(char *name, const int key,
                                 const char *uid, const int index)
{
    MenuToplevel *node = new MenuToplevel(this, name);
    int new_id = registerChild(node);
    menu_bar.insertItem(klocale->translate(name),
	node->getPopupMenu(), new_id, index);
    return node;
}

MenuNode *MenuRoot::insertLeaf(char *name, const char *command,
                               const int key, const char *uid,
                               const int index)
{
    MenuItem *item = new MenuItem(this, name, command);
    int new_id = registerChild(item);
    menu_bar.insertItem(klocale->translate(name),
	new_id, index);
    return item;
}

void MenuRoot::removeChild(const int id)
{
//    debug("MenuRoot::removeChild(%d)", id);
    MenuNode::removeChild(id);
    menu_bar.removeItem(id);
}

/* end of MenuRoot.cpp */
