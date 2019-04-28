/***************************************************************************
          MenuList.cpp  -  placeholder for a list of menu entries
                             -------------------
    begin                : Wed Dec 03 2014
    copyright            : (C) 2014 by Thomas Eschenbacher
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

#include "libkwave/String.h"

#include "libgui/MenuList.h"

//*****************************************************************************
Kwave::MenuList::MenuList(Kwave::MenuNode *parent,
                          const QString &command,
                          const QString &uid)
    :MenuNode(parent, _("LISTENTRY[") + uid + _("]"), command, 0, uid)
{
    /* NOTE: the "name" of this node is the uid of the list entries */
}

//*****************************************************************************
Kwave::MenuList::~MenuList()
{
}

//*****************************************************************************
void Kwave::MenuList::clear()
{
    Kwave::MenuNode *parent = parentNode();
    Q_ASSERT(parent);
    if (!parent) return;

    QString list_uid = name();
    Kwave::MenuNode *child;
    while ((child = parent->findUID(list_uid)) != Q_NULLPTR)
	parent->removeChild(child);
}

//*****************************************************************************
Kwave::MenuNode *Kwave::MenuList::insertLeaf(const QString &name,
                                             const QString &command,
                                             const QKeySequence &shortcut,
                                             const QString &uid)
{
    Q_UNUSED(uid)

    Kwave::MenuNode *parent = parentNode();
    Q_ASSERT(parent);
    QString list_uid = this->name();
    return (parent) ?
	parent->insertLeaf(name, command, shortcut, list_uid) : Q_NULLPTR;
}

//*****************************************************************************
//*****************************************************************************
