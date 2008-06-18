/***************************************************************************
			  MenuSub.cpp  -  submenu
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

#include <QPixmap>

#include <klocale.h>

#include "MenuItem.h"
#include "MenuSub.h"

//***************************************************************************
MenuSub::MenuSub(MenuNode *parent,
                 QMenu *menu,
                 const QString &name,
	         const QString &command,
	         const QKeySequence &shortcut,
	         const QString &uid)
    :MenuNode(parent, name, command, shortcut, uid), m_menu(menu)
{
    QAction *act = action();
    Q_ASSERT(act);
    if (act) {
	act->setText(i18n(name.toAscii()));
	if (shortcut) act->setShortcut(shortcut);
    }
}

//***************************************************************************
MenuSub::~MenuSub()
{
}

//*****************************************************************************
bool MenuSub::isEnabled()
{
    if (m_menu && !m_menu->isEnabled()) return false;
    return MenuNode::isEnabled();
}

//*****************************************************************************
void MenuSub::setEnabled(bool enable)
{
    if (m_menu) m_menu->setEnabled(enable);
}

//*****************************************************************************
const QIcon MenuSub::icon()
{
    return (m_menu) ? m_menu->icon() : QIcon();
}

//*****************************************************************************
void MenuSub::setIcon(const QIcon &icon)
{
    if (m_menu) m_menu->setIcon(icon);
}

//***************************************************************************
MenuSub *MenuSub::insertBranch(const QString &name,
                               const QString &command,
                               const QKeySequence &shortcut,
                               const QString &uid)
{
    QMenu *menu = (m_menu) ? m_menu->addMenu(name) : 0;
    Q_ASSERT(menu);
    if (!menu) return 0;

    MenuSub *sub = new MenuSub(this, menu, name, command, shortcut, uid);
    Q_ASSERT(sub);
    if (!sub) return 0;

    registerChild(sub);

    return sub;
}

//***************************************************************************
MenuNode *MenuSub::insertLeaf(const QString &name,
                              const QString &command,
                              const QKeySequence &shortcut,
                              const QString &uid)
{
    Q_ASSERT(name.length());
    Q_ASSERT(m_menu);
    if (!name.length() || !m_menu) return 0;

    MenuItem *item = new MenuItem(this, name, command, shortcut, uid);
    Q_ASSERT(item);
    if (!item) return 0;

    registerChild(item);
    m_menu->addAction(item->action());

    return item;
}

//***************************************************************************
void MenuSub::removeChild(MenuNode *child)
{
    QAction *act = action();
    if (act && m_menu) m_menu->removeAction(act);

    MenuNode::removeChild(child);
}

//***************************************************************************
bool MenuSub::specialCommand(const QString &command)
{
    Q_ASSERT(command.length());
    if (!command.length()) return false;

    if (command.startsWith("#exclusive")) {
	return true;
    } else if (command.startsWith("#number")) {
	return true;
    } else if (command.startsWith("#separator")) {
	if (m_menu) m_menu->addSeparator();
	return true;
    }

    return MenuNode::specialCommand(command);
}

//***************************************************************************
#include "MenuSub.moc"
//***************************************************************************
//***************************************************************************
