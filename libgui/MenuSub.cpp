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

//*****************************************************************************
MenuSub::MenuSub(MenuNode *parent, char *name, char *command,
                 int key, char *uid)
    :MenuItem(parent, name, command, key, uid)
{
    menu = new QPopupMenu(0, i18n(name));

    QObject::connect(menu,SIGNAL(activated(int)),
	this,SLOT(slotSelected(int)));
}

//*****************************************************************************
int MenuSub::getChildIndex(int id)
{
    return (menu) ? menu->indexOf(id) : -1;
}

//*****************************************************************************
QPopupMenu *MenuSub::getPopupMenu()
{
    return menu;
}

//*****************************************************************************
MenuNode *MenuSub::insertBranch(char *name, char *command, int key,
                                char *uid, int index)
{
    MenuSub *node = new MenuSub(this, name, command, key, uid);
    if (!node) return 0;

    if (menu) {
	int new_id = registerChild(node);
	menu->insertItem(i18n(node->getName()),
	    node->getPopupMenu(), new_id);
    }

    return node;
}

//*****************************************************************************
MenuNode *MenuSub::insertLeaf(char *name, char *command, int key,
                              char *uid, int index=-1)
{
    int new_id;
    if ((!name) || (!menu)) return 0;

    MenuItem *item = new MenuItem(this, name, command, key, uid);
    if (!item) return 0;

    new_id = registerChild(item);
    menu->insertItem(i18n(name), new_id);
    menu->setAccel(key, new_id);

    return item;
}

//*****************************************************************************
void MenuSub::removeChild(MenuNode *child)
{
    if (!child) return;
    if (children.findRef(child) == -1) return;
    menu->removeItem(child->getId());
    MenuItem::removeChild(child);
}

//*****************************************************************************
bool MenuSub::specialCommand(const char *command)
{
    if (strcmp(command, "#exclusive") == 0) {
    	// debug("MenuSub(%s) >> exclusive <<", getName());
	return true;
    } else if (strcmp(command, "#number") == 0) {
    	// debug("MenuSub(%s) >> number <<", getName());
	return true;
    } else if (strcmp(command, "#separator") == 0) {
	menu->insertSeparator(-1);
    	return true;
    }

    return MenuItem::specialCommand(command);
}

//*****************************************************************************
void MenuSub::actionChildEnableChanged(int id, bool enable)
{
    MenuNode::actionChildEnableChanged(id, enable);
    if (menu) menu->setItemEnabled(id, enable);
}

//*****************************************************************************
void MenuSub::slotSelected(int id)
{
    MenuNode *child = findChild(id);
    if (child) {
	child->actionSelected();
    } else {
	debug("MenuSub::slotSelected: child with id #%d not found!", id);
    }
}

//*****************************************************************************
void MenuSub::setItemIcon(int id, const QPixmap &icon)
{
    if (menu) menu->changeItem(icon, menu->text(id), id);
}

//*****************************************************************************
void MenuSub::setItemChecked(int id, bool enable)
{
    if ((!menu) || (!menu->findItem(id))) return;
    menu->setItemChecked(id, enable);
}

/* end of libgui/MenuSub.cpp */
