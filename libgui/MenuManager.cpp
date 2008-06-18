/***************************************************************************
        MenuManager.cpp  -  manager class for Kwave's menu structure
			     -------------------
    begin                : Sun Jun 4 2000
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
#include <stdlib.h>

#include <QKeySequence>
#include <QString>
#include <QWidget>

#include <klocale.h>

#include "libkwave/Parser.h"

#include "MenuNode.h"
#include "MenuRoot.h"
#include "MenuGroup.h"
#include "MenuManager.h"

//***************************************************************************
MenuManager::MenuManager(QWidget *parent, KMenuBar &bar)
    :QObject(parent)
{
    m_menu_root = new MenuRoot(bar);
    Q_ASSERT(m_menu_root);
    if (m_menu_root) {
	connect(
	    m_menu_root, SIGNAL(sigCommand(const QString &)),
	    this, SIGNAL(sigMenuCommand(const QString &)),
	    Qt::QueuedConnection
	);
    }
}

//***************************************************************************
void MenuManager::executeCommand(const QString &command)
{
    Q_ASSERT(command.length());
    Q_ASSERT(m_menu_root);
    if (!m_menu_root) return; // makes no sense if no menu root

    Parser parser(command);

    QKeySequence shortcut;  // keyboard shortcut (optional)
    QString id = 0;         // string id (optional)
    QString param;

    // --- 1st parameter: command to be sent when selected ---
    QString com = parser.firstParam();

    // --- 2nd parameter: position in the menu structure ---
    QString pos = parser.nextParam();

    // bail out if no menu position is found
    if (!pos.length()) {
	qWarning("no position field !");
	return ;
    }

    // --- 3rd parameter: bitmask for the key shortcut (optional) ---
    param = parser.nextParam();
    if (param.length()) shortcut =
	QKeySequence::fromString(i18n(param.toAscii()));

    // --- 4rth parameter: parse the string id of the node (optional) ---
    param = parser.nextParam();
    if (param.length()) id = param;

    // --- insert the new node into the menu structure ---
    m_menu_root->insertNode(0, pos, com, shortcut, id);
}

//***************************************************************************
void MenuManager::clearNumberedMenu(const QString &uid)
{
    Q_ASSERT(m_menu_root);
    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    if (node) node->clear();
}

//***************************************************************************
void MenuManager::addNumberedMenuEntry(const QString &uid,
	const QString &entry)
{
    Q_ASSERT(entry.length());
    if (!entry.length()) return;

    Q_ASSERT(m_menu_root);
    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    if (node) {
	QString cmd = node->command();
	QString command = cmd.contains("%1") ? cmd.arg(entry) : cmd;

	node->insertLeaf(entry, command, 0, 0);
    } else
	qWarning("MenuManager: could not find numbered Menu '%s'",
	         uid.toLocal8Bit().data());

}

//***************************************************************************
void MenuManager::selectItem(const QString &group, const QString &uid)
{
    Q_ASSERT(m_menu_root);

    if (!group.length()) {
	qWarning("MenuManager::selectItem('','%s'): no group!?",
	         uid.toLocal8Bit().data());
	return ;
    }

    if (group[0] != '@') {
	qWarning("MenuManager::selectItem('%s','%s'): "\
		"invalid group name, does not start with '@'!",
		group.toLocal8Bit().data(), uid.toLocal8Bit().data());
	return ;
    }

    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(group) : 0;
    if (!node) {
	qWarning("MenuManager::selectItem(): group '%s' not found!",
	    group.toLocal8Bit().data());
	return ;
    }

    if (!node->inherits("MenuGroup")) {
	qWarning("MenuManager::selectItem(): '%s' is not a group!",
	    group.toLocal8Bit().data());
	return ;
    }

    (reinterpret_cast<MenuGroup *>(node))->selectItem(uid);
}

//***************************************************************************
void MenuManager::setItemChecked(const QString &uid, bool check)
{
    Q_ASSERT(m_menu_root);
    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    if (node) node->setChecked(check);
}

//***************************************************************************
void MenuManager::setItemText(const QString &uid, const QString &text)
{
    Q_ASSERT(m_menu_root);
    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    if (node) node->setText(text);
}

//***************************************************************************
void MenuManager::setItemEnabled(const QString &uid, bool enable)
{
    Q_ASSERT(m_menu_root);
    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    if (node) node->setEnabled(enable);
    else qWarning("MenuManager::setItemEnabled('%s', '%d'): uid not found!",
		  uid.toLocal8Bit().data(), enable);
}

//***************************************************************************
MenuManager::~MenuManager()
{
    Q_ASSERT(m_menu_root);
    if (m_menu_root) delete m_menu_root;
}

//***************************************************************************
#include "MenuManager.moc"
//***************************************************************************
//***************************************************************************
