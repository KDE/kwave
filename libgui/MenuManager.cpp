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

#include <qapplication.h>
#include <qaccel.h>
#include <qnamespace.h>
#include <qstring.h>

#include <kapp.h>
#include <klocale.h>

#include "libkwave/Parser.h"

#include "MenuNode.h"
#include "MenuRoot.h"
#include "MenuGroup.h"
#include "MenuManager.h"

//***************************************************************************
//***************************************************************************
MenuManager::MenuManager(QWidget *parent, KMenuBar &bar)
    :QObject(parent),
    m_spx_command(this, SLOT(slotMenuCommand()))
{
    m_menu_root = new MenuRoot(bar);
    Q_ASSERT(m_menu_root);
    if (m_menu_root) {
	connect(
	    m_menu_root, SIGNAL(sigCommand(const QString &)),
	    this, SLOT(slotEnqueueCommand(const QString &))
	);
    }
}

//***************************************************************************
int MenuManager::parseToKeyCode(const QString &key_name)
{
// would be fine, but doesn't support most codes like +/-
//    return QAccel::stringToKey(key_name);

    Q_ASSERT(key_name);
    QString key = key_name;
    int keycode = 0;

    while (key.length()) {
	int pos = key.find('+');
	if (pos <= 0) pos=key.length();

	QString name = key.left(pos);
	key.remove(0, pos+1);

	// keys [A...Z]
	if (name.length() == 1) {
	    if ((name[0] >= 'A') && (name[0] <= 'Z')) {
		keycode += Key_A;
		keycode += (char)name[0].latin1() - (char)'A';
	    }
	}

	// function keys [F1...F35] ?
	if (name[0] == 'F') {
	    name.remove(0,1);
	    int nr = name.toInt();
	    if ((nr >= 1) && (nr <= 35)) {
		keycode += Key_F1 + nr - 1;
	    }
	}

	// other known keys
	if (name == "ESC")      keycode += Qt::Key_Escape;
	if (name == "PLUS")     keycode += Qt::Key_Plus;
	if (name == "MINUS")    keycode += Qt::Key_Minus;
	if (name == "SPACE")    keycode += Qt::Key_Space;
	if (name == "PAGEUP")   keycode += Qt::Key_PageUp;
	if (name == "PAGEDOWN") keycode += Qt::Key_PageDown;
	if (name == "UP")       keycode += Qt::Key_Up;
	if (name == "DEL")      keycode += Qt::Key_Delete;
	if (name == "DOWN")     keycode += Qt::Key_Down;
	if (name == "LEFT")     keycode += Qt::Key_Left;
	if (name == "RIGHT")    keycode += Qt::Key_Right;
	if (name == "HOME")     keycode += Qt::Key_Home;
	if (name == "END")      keycode += Qt::Key_End;

	if (name == "SHIFT")    keycode += Qt::SHIFT;
	if (name == "CTRL")     keycode += Qt::CTRL;
	if (name == "ALT")      keycode += Qt::ALT;
    }
    return keycode;
}

//***************************************************************************
void MenuManager::executeCommand(const QString &command)
{
    Q_ASSERT(command);
    Q_ASSERT(m_menu_root);
    if (!m_menu_root) return; // makes no sense if no menu root

    Parser parser(command);

    int key;        // keyboard shortcut (optional)
    QString id = 0; // string id (optional)
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
    key = (param.length()) ? parseToKeyCode(param) : 0;

    // --- 4rth parameter: parse the string id of the node (optional) ---
    param = parser.nextParam();
    if (param.length()) id = param;

    // --- insert the new node into the menu structure ---
    m_menu_root->insertNode(0, pos, com, key, id);
}

//***************************************************************************
void MenuManager::clearNumberedMenu(const QString &uid)
{
    Q_ASSERT(m_menu_root);
    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    if (node) node->clear();
}

//***************************************************************************
void MenuManager::slotEnqueueCommand(const QString &command)
{
    m_spx_command.enqueue(command);
}

//***************************************************************************
void MenuManager::slotMenuCommand()
{
    QString *command = m_spx_command.dequeue();
    Q_ASSERT(command);
    if (!command) return;

    emit sigMenuCommand(*command);
    delete command;
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
	QString cmd = node->getCommand();
	QString command = cmd.contains("%1") ? cmd.arg(entry) : cmd;

	node->insertLeaf(entry, command, 0, 0, -1);
    } else
	qWarning("MenuManager: could not find numbered Menu '%s'",
	         uid.local8Bit().data());

}

//***************************************************************************
void MenuManager::selectItem(const QString &group, const QString &uid)
{
    Q_ASSERT(m_menu_root);

    if (!group || !*group) {
	qWarning("MenuManager::selectItem('','%s'): no group!?",
	         uid.local8Bit().data());
	return ;
    }

    if (*group != '@') {
	qWarning("MenuManager::selectItem('%s','%s'): "\
		"invalid group name, does not start with '@'!",
		group.local8Bit().data(), uid.local8Bit().data());
	return ;
    }

    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(group) : 0;
    if (!node) {
	qWarning("MenuManager::selectItem(): group '%s' not found!",
	    group.local8Bit().data());
	return ;
    }

    if (!node->inherits("MenuGroup")) {
	qWarning("MenuManager::selectItem(): '%s' is not a group!",
	    group.local8Bit().data());
	return ;
    }

    ((MenuGroup *)node)->selectItem(uid);
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
		  uid.local8Bit().data(), enable);
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
