
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#include <qaccel.h>
#include <qnamespace.h>
#include <qstring.h>

#include <kapp.h>
#include <klocale.h>

#include <libkwave/Parser.h>

#include "MenuNode.h"
#include "MenuRoot.h"
#include "MenuGroup.h"
#include "MenuManager.h"

//***************************************************************************
//***************************************************************************
MenuManager::MenuManager(QWidget *parent, KMenuBar &bar)
    :QObject(parent)
{
    m_menu_root = new MenuRoot(bar);
    ASSERT(m_menu_root);
    if (m_menu_root) {
	connect(
	    m_menu_root, SIGNAL(sigCommand(const QString &)),
	    this, SLOT(slotMenuCommand(const QString &))
	);
    }
}

//***************************************************************************
int MenuManager::parseToKeyCode(const QString &key_name)
{

// would be fine, but doesn't support most codes like +/-
//    return QAccel::stringToKey(key_name);

    ASSERT(key_name);
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
		keycode += name[0].latin1() - 'A';
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
	if (name == "PLUS")     keycode += Key_Plus;
	if (name == "MINUS")    keycode += Key_Minus;
	if (name == "SPACE")    keycode += Key_Space;
	if (name == "CTRL")     keycode += CTRL;
	if (name == "PAGEUP")   keycode += Key_PageUp;
	if (name == "PAGEDOWN") keycode += Key_PageDown;
	if (name == "UP")       keycode += Key_Up;
	if (name == "DEL")      keycode += Key_Delete;
	if (name == "DOWN")     keycode += Key_Down;
	if (name == "LEFT")     keycode += Key_Left;
	if (name == "RIGHT")    keycode += Key_Right;
	if (name == "SHIFT")    keycode += SHIFT;
	if (name == "HOME")     keycode += Key_Home;
	if (name == "END")      keycode += Key_End;
    }

    return keycode;
}

//***************************************************************************
void MenuManager::executeCommand(const QString &command)
{
    ASSERT(command);
    ASSERT(m_menu_root);
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
	warning("no position field !");
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
    debug("MenuManager::clearNumberedMenu() --1--"); // ###
    ASSERT(m_menu_root);
    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    debug("MenuManager::clearNumberedMenu() --2--"); // ###
    if (node) node->clear();
    debug("MenuManager::clearNumberedMenu() --done--"); // ###
}

//***************************************************************************
void MenuManager::slotMenuCommand(const QString &command)
{
    debug("MenuManager::slotMenuCommand(%s)", command.data());    // ###
    emit sigMenuCommand(command);
}

//***************************************************************************
void MenuManager::addNumberedMenuEntry(const QString &uid,
	const QString &entry)
{
    ASSERT(entry.length());
    if (!entry.length()) return;

    debug("MenuManager::addNumberedMenuEntry() --1--"); // ###
    ASSERT(m_menu_root);
    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    debug("MenuManager::addNumberedMenuEntry() --2--"); // ###
    if (node) {
	QString cmd = node->getCommand();
	QString command = cmd.contains("%1") ? cmd.arg(entry) : cmd;
	node->insertLeaf(entry, command, 0, 0, -1);
    } else
	debug ("MenuManager: could not find numbered Menu '%s'", uid.data());

    debug("MenuManager::addNumberedMenuEntry() --done--"); // ###
}

//***************************************************************************
void MenuManager::selectItem(const QString &group, const QString &uid)
{
    ASSERT(m_menu_root);

    if (!group || !*group) {
	warning("MenuManager::selectItem('','%s'): no group!?", uid.data());
	return ;
    }

    if (*group != '@') {
	warning("MenuManager::selectItem('%s','%s'): "\
		"invalid group name, does not start with '@'!",
		group.data(), uid.data());
	return ;
    }

    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(group) : 0;
    if (!node) {
	warning("MenuManager::selectItem(): group '%s' not found!",
	    group.data());
	return ;
    }

    if (!node->inherits("MenuGroup")) {
	warning("MenuManager::selectItem(): '%s' is not a group!",
	    group.data());
	return ;
    }

    ((MenuGroup *)node)->selectItem(uid);
}

//***************************************************************************
void MenuManager::setItemChecked(const QString &uid, bool check)
{
    ASSERT(m_menu_root);

//    debug("MenuManager::setItemChecked('%s', %d)", uid, check);
    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    if (node) node->setChecked(check);
}

//***************************************************************************
void MenuManager::setItemEnabled(const QString &uid, bool enable)
{
    ASSERT(m_menu_root);

//    debug("MenuManager::setItemEnabled('%s', %d)", uid, enable);
    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    if (node) node->setEnabled(enable);
    else warning("MenuManager::setItemEnabled('%s', '%d'): uid not found!",
		     uid.data(), enable);
}

//***************************************************************************
MenuManager::~MenuManager()
{
    ASSERT(m_menu_root);
    if (m_menu_root) delete m_menu_root;
}

//***************************************************************************
//***************************************************************************
/* end of MenuManager.cpp */
