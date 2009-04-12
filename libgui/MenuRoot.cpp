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
/** garbage collector for menu nodes */
QList<MenuNode *> MenuRoot::m_garbage;

//***************************************************************************
MenuRoot::MenuRoot(KMenuBar &bar)
    :MenuNode(0, "(root)", 0, 0, 0), m_menu_bar(bar), m_group_list()
{
}

//***************************************************************************
MenuRoot::~MenuRoot()
{
    clear();
}

//***************************************************************************
QHash<QString, MenuGroup *> &MenuRoot::getGroupList()
{
    return m_group_list;
}

//*****************************************************************************
void MenuRoot::insertNode(const QString &name,
                          const QString &position,
                          const QString &command,
                          const QKeySequence &shortcut,
                          const QString &uid)
{
    MenuNode::insertNode(name, position, command, shortcut, uid);

    // now delete all leafes that have been converted to branches
    while (!m_garbage.isEmpty()) {
	MenuNode *node = m_garbage.takeFirst();
	if (node) delete node;
    }
}

//***************************************************************************
MenuSub *MenuRoot::insertBranch(const QString &name,
                                const QString &command,
                                const QKeySequence &shortcut,
                                const QString &uid)
{
    QMenu *menu = m_menu_bar.addMenu(name);
    Q_ASSERT(menu);
    if (!menu) return 0;

    MenuSub *sub = new MenuSub(this, menu, name, command, shortcut, uid);
    Q_ASSERT(sub);
    if (!sub) return 0;

    registerChild(sub);

    return sub;
}

//***************************************************************************
MenuNode *MenuRoot::insertLeaf(const QString &name,
                               const QString &command,
                               const QKeySequence &shortcut,
                               const QString &uid)
{
    MenuItem *item = new MenuItem(this, name, command, shortcut, uid);
    Q_ASSERT(item);
    if (!item) return 0;

    registerChild(item);
    m_menu_bar.addAction(item->action());

    return item;
}

//***************************************************************************
void MenuRoot::removeChild(MenuNode *child)
{
    Q_ASSERT(child);
    if (!child) return ;
    if (!m_children.contains(child)) return ;

    QHash<QString, MenuGroup *> &group_list = getGroupList();
    if (!group_list.contains(child->name())) {
        // only remove what has been added to the menu bar,
        // but not menu groups
	QAction *action = child->action();
	if (action) m_menu_bar.removeAction(action);
    }
    MenuNode::removeChild(child);
}

//***************************************************************************
bool MenuRoot::specialCommand(const QString &command)
{
    Q_ASSERT(command.length());
    if (!command.length()) return false;

    if (command == "#separator") {
	m_menu_bar.addSeparator();
	return true;
    }

    return MenuNode::specialCommand(command);
}

//***************************************************************************
void MenuRoot::deleteLater(MenuNode *node)
{
    if (node) m_garbage.append(node);
}

//***************************************************************************
#include "MenuRoot.moc"
//***************************************************************************
//***************************************************************************
