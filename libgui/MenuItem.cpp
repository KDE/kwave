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

#include <qpixmap.h>
#include <qpopupmenu.h>
#include <kapp.h>
#include <kiconloader.h>

#include "libkwave/Parser.h"

#include "MenuNode.h"
#include "MenuSub.h"
#include "MenuGroup.h"
#include "MenuItem.h"

//*****************************************************************************
MenuItem::MenuItem(MenuNode *parent, char *name, char *command,
                   int key, char *uid)
  :MenuNode(parent, name, command, key, uid)
{
    this->checkable = false;
    exclusive_group=0;
}

//*****************************************************************************
MenuItem::~MenuItem()
{
    if (exclusive_group) {
	delete exclusive_group;
	exclusive_group=0;
    }
}

//*****************************************************************************
void MenuItem::actionSelected()
{
    MenuGroup *group=0;

    if (isCheckable()) {
	if (exclusive_group) {
	    MenuNode *root = getRootNode();
	    if (root) group = (MenuGroup*)root->findUID(exclusive_group);
	}

	if (group && ((MenuNode*)group)->inherits("MenuGroup")) {
	    // exclusive check == selection
	    group->selectItem(getUID());
	} else {
	    // normal check, maybe multiple
	    setChecked(true);
	}
    }

    MenuNode::actionSelected();
}

//*****************************************************************************
int MenuItem::getIndex()
{
    MenuNode *parent = getParentNode();
    return (parent) ? parent->getChildIndex(getId()) : -1;
}

//*****************************************************************************
bool MenuItem::specialCommand(const char *command)
{
    if (strncmp(command, "#icon(", 6) == 0) {
	// --- give the item an icon ---
	Parser parser(command);
	const char *filename = parser.getFirstParam();
	if (filename) {
	    setIcon(Icon(filename));
	}
	return true;
    } else if (strcmp(command, "#listmenu") == 0) {
	// insert an empty submenu for the list items
	MenuNode *parent = getParentNode();
	if (parent) parent->leafToBranch(this);
    	
	return true;
    } else if (strcmp(command, "#checkable") == 0) {
    	// checking/selecting of the item (non-exclusive)
	setCheckable(true);
    } else if (strncmp(command,"#exclusive(",11) == 0) {
	Parser parser(command);
	const char *group;

	// join to a list of groups
	group = parser.getFirstParam();
	while (group) {
	    if (!exclusive_group) {
		exclusive_group=duplicateString(group);
		joinGroup(group);
	    } else if (strcmp(exclusive_group, group)!=0) {
		warning("menu item '%s' already member of "\
			"exclusive group '%s'", getName(),
			exclusive_group);
	    }
	    group = parser.getNextParam();
	}
	
	// make the item checkable
	setCheckable(true);
	return true;
    }

    return (MenuNode::specialCommand(command));
}
	
//*****************************************************************************
bool MenuItem::isCheckable()
{
    return checkable;
}

//*****************************************************************************
void MenuItem::setCheckable(bool checkable)
{
    MenuNode *parent = getParentNode();
    if (parent && parent->inherits("MenuSub")) {
	QPopupMenu *popup = ((MenuSub*)parent)->getPopupMenu();
	if (popup) popup->setCheckable(checkable);
    }

    this->checkable = checkable;
}

//*****************************************************************************
void MenuItem::setChecked(bool check)
{
    MenuNode *parent = getParentNode();
    if (parent) parent->setItemChecked(getId(), check);
}

/* end of libgui/MenuItem.cpp */
