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

#include <QListIterator>

#include <KLocalizedString>
#include <TODO:kmenubar.h>

#include "libkwave/String.h"

#include "libgui/MenuItem.h"
#include "libgui/MenuRoot.h"
#include "libgui/MenuSub.h"

//***************************************************************************
/** garbage collector for menu nodes */
QList<Kwave::MenuNode *> Kwave::MenuRoot::m_garbage;

//***************************************************************************
Kwave::MenuRoot::MenuRoot(KMenuBar &bar)
    :Kwave::MenuNode(0, _("(root)"), QString(), 0, QString()),
     m_menu_bar(bar), m_group_list()
{
}

//***************************************************************************
Kwave::MenuRoot::~MenuRoot()
{
    clear();
}

//***************************************************************************
QHash<QString, Kwave::MenuGroup *> &Kwave::MenuRoot::groupList()
{
    return m_group_list;
}

//*****************************************************************************
void Kwave::MenuRoot::insertNode(const QString &name,
                                 const QString &position,
                                 const QString &command,
                                 const QKeySequence &shortcut,
                                 const QString &uid)
{
    Kwave::MenuNode::insertNode(name, position, command, shortcut, uid);

    // now delete all leafs that have been converted to branches
    while (!m_garbage.isEmpty()) {
	Kwave::MenuNode *node = m_garbage.takeFirst();
	if (node) delete node;
    }
}

//***************************************************************************
Kwave::MenuSub *Kwave::MenuRoot::insertBranch(const QString &name,
                                              const QString &command,
                                              const QKeySequence &shortcut,
                                              const QString &uid)
{
    QMenu *menu = m_menu_bar.addMenu(name);
    Q_ASSERT(menu);
    if (!menu) return 0;

    Kwave::MenuSub *sub =
	new Kwave::MenuSub(this, menu, name, command, shortcut, uid);
    Q_ASSERT(sub);
    if (!sub) return 0;

    insertChild(sub, 0);

    return sub;
}

//***************************************************************************
Kwave::MenuNode *Kwave::MenuRoot::insertLeaf(const QString &name,
                                             const QString &command,
                                             const QKeySequence &shortcut,
                                             const QString &uid)
{
    Kwave::MenuItem *item =
	new Kwave::MenuItem(this, name, command, shortcut, uid);
    Q_ASSERT(item);
    if (!item) return 0;

    insertChild(item, 0);
    m_menu_bar.addAction(item->action());

    return item;
}

//***************************************************************************
void Kwave::MenuRoot::hideChild(Kwave::MenuSub *child)
{
    Q_ASSERT(child);
    if (!child) return;
    if (!m_children.contains(child)) return;
    if (groupList().contains(child->name())) return;

    QAction *action = child->action();
    if (action) m_menu_bar.removeAction(action);
}

//***************************************************************************
void Kwave::MenuRoot::showChild(Kwave::MenuSub *child)
{
    Q_ASSERT(child);
    if (!child) return;
    if (!m_children.contains(child)) return;
    if (groupList().contains(child->name())) return;

    // find the menu bar entry after which we can insert
    QAction *action_before = 0;
    QListIterator<Kwave::MenuNode *> it(m_children);
    it.toBack();
    while (it.hasPrevious()) {
	Kwave::MenuNode *c = it.previous();
	if (c == child) break;
	if (c) action_before = c->action();
    }

    if (action_before)
	m_menu_bar.insertMenu(action_before, child->menu());
    else
	m_menu_bar.addMenu(child->menu());
}

//***************************************************************************
void Kwave::MenuRoot::removeChild(Kwave::MenuNode *child)
{
    Q_ASSERT(child);
    if (!child) return;
    if (!m_children.contains(child)) return;

    QHash<QString, Kwave::MenuGroup *> &group_list = groupList();
    if (!group_list.contains(child->name())) {
        // only remove what has been added to the menu bar,
        // but not menu groups
	QAction *action = child->action();
	if (action) m_menu_bar.removeAction(action);
    }
    Kwave::MenuNode::removeChild(child);
}

//***************************************************************************
bool Kwave::MenuRoot::specialCommand(const QString &command)
{
    Q_ASSERT(command.length());
    if (!command.length()) return false;

    if (command == _("#separator")) {
	m_menu_bar.addSeparator();
	return true;
    }

    return Kwave::MenuNode::specialCommand(command);
}

//***************************************************************************
void Kwave::MenuRoot::deleteLater(Kwave::MenuNode *node)
{
    if (node) m_garbage.append(node);
}

//***************************************************************************
//***************************************************************************
//***************************************************************************
