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

#include <QPixmap>
#include <QMenu>

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>

#include "libkwave/Parser.h"

#include "MenuNode.h"
#include "MenuSub.h"
#include "MenuGroup.h"
#include "MenuItem.h"

//*****************************************************************************
MenuItem::MenuItem(MenuNode *parent,
                   const QString &name,
                   const QString &command,
                   const QKeySequence &shortcut,
                   const QString &uid)
    :MenuNode(parent, name, command, shortcut, uid),
     m_exclusive_group(), m_action(0)
{
    Q_ASSERT(parent);
    if (!parent) return;

    m_action.setText(i18n(name.toUtf8()));
    if (shortcut) m_action.setShortcut(shortcut);

    connect(&m_action, SIGNAL(triggered(bool)),
	    this, SLOT(actionTriggered(bool)));
}

//*****************************************************************************
MenuItem::~MenuItem()
{
}

//*****************************************************************************
void MenuItem::actionTriggered(bool checked)
{
    Q_UNUSED(checked);
    actionSelected();
}

//*****************************************************************************
void MenuItem::actionSelected()
{
    MenuGroup *group = 0;

    if (isCheckable()) {
	if (m_exclusive_group.length()) {
	    MenuNode *root = getRootNode();
	    if (root) group =
		static_cast<MenuGroup *>(root->findUID(m_exclusive_group));
	}

	if (group && (static_cast<MenuNode *>(group))->inherits("MenuGroup")) {
	    // exclusive check == selection
	    group->selectItem(uid());
	} else {
	    // normal check, maybe multiple
	    setChecked(true);
	}
    }

    MenuNode::actionSelected();
}

//*****************************************************************************
bool MenuItem::specialCommand(const QString &command)
{

    if (command.startsWith("#checkable")) {
	// checking/selecting of the item (non-exclusive)
	setCheckable(true);
    }

    if (command.startsWith("#exclusive(")) {
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
			name().toLocal8Bit().data(),
			m_exclusive_group.toLocal8Bit().data());
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
bool MenuItem::isEnabled()
{
    if (!m_action.isEnabled()) return false;
    return MenuNode::isEnabled();
}

//*****************************************************************************
void MenuItem::setEnabled(bool enable)
{
    m_action.setEnabled(enable);
}

//*****************************************************************************
bool MenuItem::isCheckable()
{
    return m_action.isCheckable();
}

//*****************************************************************************
void MenuItem::setCheckable(bool checkable)
{
    m_action.setCheckable(checkable);
}

//*****************************************************************************
void MenuItem::setChecked(bool check)
{
    m_action.setChecked(check);
}

//*****************************************************************************
void MenuItem::setText(const QString &text)
{
    m_action.setText(text);
}

//*****************************************************************************
const QIcon MenuItem::icon()
{
    return m_action.icon();
}

//*****************************************************************************
void MenuItem::setIcon(const QIcon &icon)
{
    m_action.setIcon(icon);
}

//***************************************************************************
#include "MenuItem.moc"
//*****************************************************************************
//*****************************************************************************
