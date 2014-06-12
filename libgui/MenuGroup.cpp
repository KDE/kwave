/***************************************************************************
	  MenuGroup.cpp  - controls a group of menu nodes
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

#include <QtCore/QHash>
#include <QtCore/QObject>

#include <QtGui/QActionGroup>

#include "libgui/MenuNode.h"
#include "libgui/MenuGroup.h"

//*****************************************************************************
Kwave::MenuGroup::MenuGroup(Kwave::MenuNode *parent,
                            const QString &name,
                            Kwave::MenuGroup::Mode mode)
    :m_parent(parent),
     m_name(name),
     m_members(),
     m_action_group((mode == EXCLUSIVE) ? new QActionGroup(parent) : 0),
     m_enabled(true)
{
    Q_ASSERT(parent);
    Q_ASSERT(m_name.length());

    // register this group in the top level group list
    QHash<QString, Kwave::MenuGroup *> &group_list = m_parent->groupList();
    if (!group_list.contains(m_name))
	group_list[m_name] = this;

    if (m_action_group)
	m_action_group->setExclusive(true);
}

//*****************************************************************************
Kwave::MenuGroup::~MenuGroup()
{
    clear();

    // de-register this group from the top level group list
    QHash<QString, Kwave::MenuGroup *> &group_list = m_parent->groupList();
    if (group_list.contains(m_name))
	group_list.remove(m_name);
}

//*****************************************************************************
void Kwave::MenuGroup::join(Kwave::MenuNode *node)
{
    if (node && !m_members.contains(node)) {
	m_members.append(node);
	if (m_action_group && node->action())
	    m_action_group->addAction(node->action());
    }
}

//*****************************************************************************
void Kwave::MenuGroup::leave(Kwave::MenuNode *node)
{
    if (node && m_members.contains(node)) {
	m_members.removeAll(node);
	if (m_action_group && node->action())
	    m_action_group->removeAction(node->action());
    }
}

//*****************************************************************************
void Kwave::MenuGroup::setEnabled(bool enable)
{
    foreach (Kwave::MenuNode *member, m_members) {
	if (member) member->setEnabled(enable);
    }
}

//*****************************************************************************
void Kwave::MenuGroup::selectItem(const QString &uid)
{
    Kwave::MenuNode *new_selection = 0;

    foreach (Kwave::MenuNode *member, m_members) {
	if (member && (uid == member->uid()))
	    new_selection = member;    // new selected member found !
	else
	    member->setChecked(false); // remove check from others
    }

    // select the new one if found
    if (new_selection) new_selection->setChecked(true);

}

//*****************************************************************************
void Kwave::MenuGroup::clear()
{
    // de-register all member nodes from us
    while (!m_members.isEmpty())
	leave(m_members.first());
}

//***************************************************************************
//***************************************************************************
