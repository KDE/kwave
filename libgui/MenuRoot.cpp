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

#include "config.h"
#include <klocale.h>
#include <kmenubar.h>

#include "MenuItem.h"
#include "MenuSub.h"
#include "MenuRoot.h"

//***************************************************************************
MenuRoot::MenuRoot(KMenuBar &bar)
    :MenuNode(0, "(root)"),
    m_menu_bar(bar),
    m_group_list()
{
}

//***************************************************************************
MenuRoot::~MenuRoot()
{
    clear();
}

//***************************************************************************
int MenuRoot::getChildIndex(int id)
{
    for (unsigned int i = 0; i < m_menu_bar.count(); i++) {
	if (m_menu_bar.idAt(i) == id) return i;
    }
    return -1;
}

//***************************************************************************
QDict<MenuNode> *MenuRoot::getGroupList()
{
    return &m_group_list;
}

//***************************************************************************
MenuNode *MenuRoot::insertBranch(const QString &name, const QString &command,
                                 int key, const QString &uid, int index)
{
    MenuSub *node = new MenuSub(this, name, command, key, uid);
    Q_ASSERT(node);
    if (!node) return 0;

    int new_id = registerChild(node);
    m_menu_bar.insertItem(i18n(name), &(node->getPopupMenu()), new_id, index);
    return node;
}

//***************************************************************************
MenuNode *MenuRoot::insertLeaf(const QString &name, const QString &command,
			       int key, const QString &uid,
			       int index)
{
    MenuItem *item = new MenuItem(this, name, command, key, uid);
    Q_ASSERT(item);
    if (!item) return 0;

    int new_id = registerChild(item);
    m_menu_bar.insertItem(i18n(name), new_id, index);
    return item;
}

//***************************************************************************
void MenuRoot::removeChild(MenuNode *child)
{
    Q_ASSERT(child);
    if (!child) return ;
    if (m_children.findRef(child) == -1) return ;

    QDict<MenuNode> *group_list = getGroupList();
    if (!group_list || (group_list->find(child->getName()) == 0)) {
        // only remove what has been added to the menu bar,
        // but not menu groups
        m_menu_bar.removeItem(child->getId());
    }
    MenuNode::removeChild(child);
}

//***************************************************************************
void MenuRoot::actionChildEnableChanged(int /*id*/, bool /*enable*/)
{
    // do nothing, the child nodes of the toplevel menu have already
    // been enabled/disabled
    // we don't want to -> "menu_bar.setItemEnabled(id, enable);" !!!
}

//***************************************************************************
bool MenuRoot::specialCommand(const QString &command)
{
    Q_ASSERT(command);
    if (!command) return false;

    if (strcmp(command, "#separator") == 0) {
	m_menu_bar.insertSeparator();
	return true;
    }

    return MenuNode::specialCommand(command);
}

//***************************************************************************
#include "MenuRoot.moc"
//***************************************************************************
//***************************************************************************
