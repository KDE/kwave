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


#include <new>

#include <QPixmap>

#include <KLocalizedString>

#include "libkwave/String.h"

#include "libgui/MenuItem.h"
#include "libgui/MenuRoot.h"
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
    if (act) act->setText(i18nc(UTF8(_("menu: ") + path()), UTF8(name)));
}

//***************************************************************************
Kwave::MenuSub::~MenuSub()
{
}

//*****************************************************************************
void Kwave::MenuSub::setVisible(bool visible)
{
    if (!m_menu) return;

    Kwave::MenuRoot *root = qobject_cast<Kwave::MenuRoot *>(rootNode());
    Q_ASSERT(root);
    if (root && (parentNode() == root)) {
        // special case: entries of the main menu can only be made
        // visible/invisible by adding to/removing from the main menu
        if (visible) {
            root->showChild(this);
        }
        else
            root->hideChild(this);
    } else {
        // normal menu entry
        m_menu->setVisible(visible);
    }
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
    QMenu *menu = (m_menu) ? m_menu->addMenu(name) : Q_NULLPTR;
    Q_ASSERT(menu);
    if (!menu) return Q_NULLPTR;

    Kwave::MenuSub *sub = new(std::nothrow)
        Kwave::MenuSub(this, menu, name, command, shortcut, uid);
    Q_ASSERT(sub);
    if (!sub) return Q_NULLPTR;

    insertChild(sub, Q_NULLPTR);

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
    if (!name.length() || !m_menu) return Q_NULLPTR;

    Kwave::MenuItem *item = new(std::nothrow)
        Kwave::MenuItem(this, name, command, shortcut, uid);
    Q_ASSERT(item);
    if (!item) return Q_NULLPTR;

    /*
     * find out where to insert the leaf: if there is a placeholder
     * with the matching uid, insert after that one, otherwise append
     * to the end
     */
    bool found = false;
    Kwave::MenuNode *child_after = Q_NULLPTR;
    QListIterator<Kwave::MenuNode *> it(m_children);
    it.toBack();
    while (!found && it.hasPrevious()) {
        Kwave::MenuNode *child = it.previous();
        if (!child) continue;
        if (uid.length() && ((uid == child->uid()) || (uid == child->name())))
            found = true;
        else if (child->action())
            child_after = child;
    }

    insertChild(item, (found) ? child_after : Q_NULLPTR);

    QAction *action_after = (found && child_after) ?
        child_after->action() : Q_NULLPTR;
    if (action_after)
        m_menu->insertAction(action_after, item->action());
    else
        m_menu->addAction(item->action());

    return item;
}

//***************************************************************************
void Kwave::MenuSub::removeChild(Kwave::MenuNode *child)
{
    QAction *act = (child) ? child->action() : Q_NULLPTR;
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
    } else if (command.startsWith(_("#separator"))) {
        if (m_menu) m_menu->addSeparator();
        return true;
    }

    return Kwave::MenuNode::specialCommand(command);
}

//***************************************************************************
//***************************************************************************

#include "moc_MenuSub.cpp"
