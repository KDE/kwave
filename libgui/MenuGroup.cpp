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
#include <stdio.h>

#include <QHash>
#include <QObject>

#include <kapplication.h>

#include "MenuNode.h"
#include "MenuGroup.h"

//*****************************************************************************
Kwave::MenuGroup::MenuGroup(Kwave::MenuNode *parent, const QString &name)
    :Kwave::MenuNode(parent, name, 0, 0, name)
{
    if (parent) parent->registerChild(this);
}

//*****************************************************************************
Kwave::MenuGroup::~MenuGroup()
{
    clear();

    QHash<QString, Kwave::MenuGroup *> &group_list = getGroupList();
    const QString key = name();
    if (group_list.contains(key)) {
	group_list.remove(key);
    }
}

//*****************************************************************************
void Kwave::MenuGroup::setEnabled(bool enable)
{
    foreach (Kwave::MenuNode *child, m_children) {
	if (child) child->setEnabled(enable);
    }
}

//*****************************************************************************
void Kwave::MenuGroup::selectItem(const QString &uid)
{
    Kwave::MenuNode *new_selection = 0;

    foreach (Kwave::MenuNode *child, m_children) {
	if (child && (uid == child->uid()))
	    new_selection = child;    // new selected child found !
	else
	    child->setChecked(false);    // remove check from others
    }

    // select the new one if found
    if (new_selection) new_selection->setChecked(true);

}

//*****************************************************************************
void Kwave::MenuGroup::clear()
{
    // deregister all child nodes from us
    while (!m_children.isEmpty())
	removeChild(m_children.first());
}

//***************************************************************************
#include "MenuGroup.moc"
//***************************************************************************
//***************************************************************************
