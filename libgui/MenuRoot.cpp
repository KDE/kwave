/***************************************************************************
                          MenuRoot.cpp  -  description
                             -------------------
    begin                : Mon Jan 10 2000
    copyright            : (C) 2000 by Martin Wilz
    email                : mwilz@ernie.MI.Uni-Koeln.DE
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kmenubar.h>

#include "MenuRoot.h"

/**
 * Constructor.
 * @param bar reference to a KMenuBar
 */
MenuRoot::MenuRoot(KMenuBar &bar)
    :MenuNode(0), menu_bar(bar)
{
}

/**
 * Inserts a menu node into the menu bar (if not null)
 * @param node pointer to a MenuNode object, normally this is a
 *             toplevel menu.
 * @return the unique id of the item or -1 on errors
 */
int MenuRoot::insertNode(MenuNode *node)
{
    if (!node) return -1;

    int new_id = MenuNode::insertNode(node);
    menu_bar.insertItem(node->getName(), new_id);

    return new_id;
}

/* end of MenuRoot.cpp */
