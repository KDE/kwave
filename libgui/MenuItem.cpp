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

#include "config.h"
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
MenuItem::MenuItem(MenuNode *parent, const QString &name,
	const QString &command, int key, const QString &uid)
    :MenuNode(parent, name, command, key, uid),
    m_checkable(false),
    m_exclusive_group(0),
    m_text(name)
{
}

//*****************************************************************************
MenuItem::~MenuItem()
{
}

//*****************************************************************************
void MenuItem::actionSelected()
{
    MenuGroup *group = 0;

    if (isCheckable()) {
	if (m_exclusive_group.length()) {
	    MenuNode *root = getRootNode();
	    if (root) group = (MenuGroup*)root->findUID(m_exclusive_group);
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
bool MenuItem::specialCommand(const QString &command)
{
    if (command.startsWith("#icon(")) {
	// --- give the item an icon ---
	Parser parser(command);
	const QString &filename = parser.firstParam();
	if (filename.length()) {
	    // try to load from standard dirs
	    static KIconLoader loader;
	    QPixmap icon = loader.loadIcon(filename,
		KIcon::Small,0,KIcon::DefaultState,0L,true);

	    if (!icon.isNull()) {
		setIcon(icon);
	    } else {
		qWarning("MenuItem '%s': icon '%s' not found !",
		    name(), filename.local8Bit().data());
	    }
	}
	return true;
    } else if (command.startsWith("#listmenu")) {
	// insert an empty submenu for the list items
	MenuNode *parent = getParentNode();
	if (parent) parent->leafToBranch(this);

	return true;
    } else if (command.startsWith("#checkable")) {
	// checking/selecting of the item (non-exclusive)
	setCheckable(true);
    } else if (command.startsWith("#exclusive(")) {
	Parser parser(command);

	// join to a list of groups
	QString group = parser.firstParam();
	while (group.length()) {
	    if (!m_exclusive_group.length()) {
		m_exclusive_group = group;
		joinGroup(group);
	    } else if (m_exclusive_group != group) {
		qWarning("menu item '%s' already member of "\
			"exclusive group '%s'",
			getName().local8Bit().data(),
			m_exclusive_group.local8Bit().data());
	    }
	    group = parser.nextParam();
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
    return m_checkable;
}

//*****************************************************************************
void MenuItem::setCheckable(bool checkable)
{
    MenuNode *parent = getParentNode();
    if (parent && parent->inherits("MenuSub")) {
	QPopupMenu &popup = ((MenuSub*)parent)->getPopupMenu();
	popup.setCheckable(checkable);
    }

    m_checkable = checkable;
}

//*****************************************************************************
void MenuItem::setChecked(bool check)
{
    MenuNode *parent = getParentNode();
    if (parent) parent->setItemChecked(getId(), check);
}

//*****************************************************************************
void MenuItem::setText(const QString &text)
{
    if (text == m_text) return; // no change
    m_text = text;

    MenuNode *parent = getParentNode();
    if (parent && parent->inherits("MenuSub")) {
	QPopupMenu &popup = ((MenuSub*)parent)->getPopupMenu();
	popup.changeItem(getId(), m_text);
    }
}

//*****************************************************************************
//*****************************************************************************
/* end of libgui/MenuItem.cpp */
