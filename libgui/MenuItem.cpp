/***************************************************************************
                          MenuItem.h  -  description
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

#include "MenuNode.h"
#include "MenuItem.h"

MenuItem::MenuItem(const char *command, const char *name)
  :MenuNode(command, name)
{
}

int MenuItem::getIndex()
{
    MenuNode *parent = getParentNode();
    return (parent) ? parent->getChildIndex(getId()) : -1;
}
