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

#include <QtGui/QPixmap>
#include <QtGui/QMenu>

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>

#include "libkwave/Parser.h"
#include "libkwave/String.h"

#include "libgui/MenuNode.h"
#include "libgui/MenuGroup.h"
#include "libgui/MenuItem.h"

//*****************************************************************************
Kwave::MenuItem::MenuItem(Kwave::MenuNode *parent,
                          const QString &name,
                          const QString &command,
                          const QKeySequence &shortcut,
                          const QString &uid)
    :Kwave::MenuNode(parent, name, command, shortcut, uid),
     m_exclusive_group(), m_action(0)
{
    Q_ASSERT(parent);
    if (!parent) return;

    m_action.setText(i18n(__(name)));
    if (shortcut) m_action.setShortcut(shortcut);

    connect(&m_action, SIGNAL(triggered(bool)),
	    this, SLOT(actionTriggered(bool)));
}

//*****************************************************************************
Kwave::MenuItem::~MenuItem()
{
}

//*****************************************************************************
void Kwave::MenuItem::actionTriggered(bool checked)
{
    Q_UNUSED(checked);
    actionSelected();
}

//*****************************************************************************
void Kwave::MenuItem::actionSelected()
{
    if (isCheckable() && !m_exclusive_group.length())
	setChecked(true);

    Kwave::MenuNode::actionSelected();
}

//*****************************************************************************
bool Kwave::MenuItem::specialCommand(const QString &command)
{
    Kwave::Parser parser(command);

    if (command == _("#checkable")) {
	// checking/selecting of the item (non-exclusive)
	setCheckable(true);
    }

    if (parser.command() == _("#exclusive")) {
	// join to a list of groups
	QString group = parser.firstParam();
	while (group.length()) {
	    if (!m_exclusive_group.length()) {
		m_exclusive_group = group;
		joinGroup(group, Kwave::MenuGroup::EXCLUSIVE);
	    } else if (m_exclusive_group != group) {
		qWarning("menu item '%s' already member of "
			"exclusive group '%s'",
			DBG(name()), DBG(m_exclusive_group));
	    }
	    group = parser.nextParam();
	}

	// make the item checkable
	setCheckable(true);
	return true;
    }

    return (Kwave::MenuNode::specialCommand(command));
}

//*****************************************************************************
bool Kwave::MenuItem::isEnabled()
{
    if (!m_action.isEnabled()) return false;
    return Kwave::MenuNode::isEnabled();
}

//*****************************************************************************
void Kwave::MenuItem::setEnabled(bool enable)
{
    m_action.setEnabled(enable);
}

//*****************************************************************************
bool Kwave::MenuItem::isCheckable()
{
    return m_action.isCheckable();
}

//*****************************************************************************
void Kwave::MenuItem::setCheckable(bool checkable)
{
    m_action.setCheckable(checkable);
}

//*****************************************************************************
void Kwave::MenuItem::setChecked(bool check)
{
    m_action.setChecked(check);
}

//*****************************************************************************
void Kwave::MenuItem::setText(const QString &text)
{
    m_action.setText(text);
}

//*****************************************************************************
const QIcon Kwave::MenuItem::icon()
{
    return m_action.icon();
}

//*****************************************************************************
void Kwave::MenuItem::setIcon(const QIcon &icon)
{
    m_action.setIcon(icon);
}

//***************************************************************************
#include "MenuItem.moc"
//*****************************************************************************
//*****************************************************************************
