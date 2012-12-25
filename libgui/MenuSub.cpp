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

#include <QtGui/QPixmap>

#include <klocale.h>

#include "libkwave/String.h"

#include "libgui/MenuItem.h"
#include "libgui/MenuSub.h"

//***************************************************************************
Kwave::MenuSub::MenuSub(Kwave::MenuNode *parent,
                        QMenu *menu,
                        const QString &name,
                        const QString &command,
                        const QKeySequence &shortcut,
                        const QString &uid)
    :Kwave::MenuNode(parent, name, command, shortcut, uid), m_menu(menu)
{
    QAction *act = action();
    Q_ASSERT(act);
    if (act) act->setText(i18n(__(name)));
}

//***************************************************************************
Kwave::MenuSub::~MenuSub()
{
}

//*****************************************************************************
bool Kwave::MenuSub::isEnabled()
{
    if (m_menu && !m_menu->isEnabled()) return false;
    return Kwave::MenuNode::isEnabled();
}

//*****************************************************************************
void Kwave::MenuSub::setEnabled(bool enable)
{
    if (m_menu) m_menu->setEnabled(enable);
}

//*****************************************************************************
const QIcon Kwave::MenuSub::icon()
{
    return (m_menu) ? m_menu->icon() : QIcon();
}

//*****************************************************************************
void Kwave::MenuSub::setIcon(const QIcon &icon)
{
    if (m_menu) m_menu->setIcon(icon);
}

//***************************************************************************
Kwave::MenuSub *Kwave::MenuSub::insertBranch(const QString &name,
                                             const QString &command,
                                             const QKeySequence &shortcut,
                                             const QString &uid)
{
    QMenu *menu = (m_menu) ? m_menu->addMenu(name) : 0;
    Q_ASSERT(menu);
    if (!menu) return 0;

    Kwave::MenuSub *sub =
	new Kwave::MenuSub(this, menu, name, command, shortcut, uid);
    Q_ASSERT(sub);
    if (!sub) return 0;

    registerChild(sub);

    return sub;
}

//***************************************************************************
Kwave::MenuNode *Kwave::MenuSub::insertLeaf(const QString &name,
                                            const QString &command,
                                            const QKeySequence &shortcut,
                                            const QString &uid)
{
    Q_ASSERT(name.length());
    Q_ASSERT(m_menu);
    if (!name.length() || !m_menu) return 0;

    Kwave::MenuItem *item =
	new Kwave::MenuItem(this, name, command, shortcut, uid);
    Q_ASSERT(item);
    if (!item) return 0;

    registerChild(item);
    m_menu->addAction(item->action());

    return item;
}

//***************************************************************************
void Kwave::MenuSub::removeChild(Kwave::MenuNode *child)
{
    QAction *act = action();
    if (act && m_menu) m_menu->removeAction(act);

    Kwave::MenuNode::removeChild(child);
}

//***************************************************************************
bool Kwave::MenuSub::specialCommand(const QString &command)
{
    Q_ASSERT(command.length());
    if (!command.length()) return false;

    if (command.startsWith(_("#exclusive"))) {
	return true;
    } else if (command.startsWith(_("#number"))) {
	return true;
    } else if (command.startsWith(_("#separator"))) {
	if (m_menu) m_menu->addSeparator();
	return true;
    }

    return Kwave::MenuNode::specialCommand(command);
}

//***************************************************************************
#include "MenuSub.moc"
//***************************************************************************
//***************************************************************************
