
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#include <qkeycode.h>

#include <kapp.h>
#include <klocale.h>

#include <libkwave/Parser.h>
#include <libkwave/String.h>

#include "MenuNode.h"
#include "MenuRoot.h"
#include "MenuGroup.h"
#include "MenuManager.h"

//*****************************************************************************
MenuManager::MenuManager(QWidget *parent, KMenuBar &bar)
    :QObject(parent)
{
    menu_root = new MenuRoot(bar);
    ASSERT(menu_root);
    if (menu_root) {
	connect(
	    menu_root, SIGNAL(sigCommand(const char*)),
	    this, SLOT(slotMenuCommand(const char*))
	);
    }
}

//*****************************************************************************
int MenuManager::parseToKeyCode(const char *key_name)
{
    ASSERT(key_name);
    char *key = duplicateString(key_name);
    ASSERT(key);
    int keycode = 0;
    int len;
    int cnt = 0;

    len = (key) ? strlen(key) : 0;
    if (len == 1) {
	keycode = Key_A + key[0]-'A';
	delete[] key;
	return keycode;
    }

    while (cnt < len) {
	int pos = cnt;
	while ((key[cnt]) && (key[cnt] != '+')) cnt++;
	if (cnt < len) key[cnt] = 0;

	// keys [A...Z]
	if (strlen(&key[pos]) == 1) {
	    if ((key[pos] >= 'A') && (key[pos] <= 'Z'))
		keycode += Key_A + key[pos] - 'A';
	}

	// function keys [F1...F35] ?
	if (key[pos] == 'F') {
	    int nr = atoi(key + pos + 1);
	    if ((nr >= 1) && (nr <= 35)) {
		keycode += Key_F1 + nr - 1;
	    }
	}

	// other known keys
	if (!strcmp(&key[pos], "PLUS")) keycode += Key_Plus;
	if (!strcmp(&key[pos], "MINUS")) keycode += Key_Minus;
	if (!strcmp(&key[pos], "SPACE")) keycode += Key_Space;
	if (!strcmp(&key[pos], "CTRL")) keycode += CTRL;
	if (!strcmp(&key[pos], "PAGEUP")) keycode += Key_PageUp;
	if (!strcmp(&key[pos], "PAGEDOWN")) keycode += Key_PageDown;
	if (!strcmp(&key[pos], "UP")) keycode += Key_Up;
	if (!strcmp(&key[pos], "DEL")) keycode += Key_Delete;
	if (!strcmp(&key[pos], "DOWN")) keycode += Key_Down;
	if (!strcmp(&key[pos], "LEFT")) keycode += Key_Left;
	if (!strcmp(&key[pos], "RIGHT")) keycode += Key_Right;
	if (!strcmp(&key[pos], "SHIFT")) keycode += SHIFT;
	if (!strcmp(&key[pos], "HOME")) keycode += Key_Home;
	if (!strcmp(&key[pos], "END")) keycode += Key_End;

	cnt++;
    }

    if (key) delete[] key;
    return keycode;
}

//*****************************************************************************
void MenuManager::executeCommand(const char *command)
{
    ASSERT(command);
    ASSERT(menu_root);

    Parser parser(command);

    const char *tmp;
    char *com = 0;
    char *pos = 0;
    int key;      // keyboard shortcut (optional)
    char *id = 0;     // string id (optional)

    // --- 1st parameter: command to be sent when selected ---
    tmp = parser.getFirstParam();
    if (tmp) com = duplicateString (tmp);

    // --- 2nd parameter: position in the menu structure ---
    tmp = parser.getNextParam();
    if (tmp) pos = duplicateString (tmp);

    // bail out if no menu position is found
    if (!pos) {
	debug ("no position field !\n");
	if (com) delete[] com;
	return ;
    }

    // --- 3rd parameter: bitmask for the key shortcut (optional) ---
    tmp = parser.getNextParam();
    key = (tmp) ? parseToKeyCode(tmp) : 0;

    // --- 4rth parameter: parse the string id of the node (optional) ---
    tmp = parser.getNextParam();
    if (tmp) id = duplicateString(tmp);

    // --- insert the new node into the menu structure ---
    if (menu_root) menu_root->insertNode(0, pos, com, key, id);

    if (com) delete[] com;
    if (pos) delete[] pos;
    if (id) delete[] id;
}

//*****************************************************************************
void MenuManager::clearNumberedMenu (const char *uid)
{
    ASSERT(menu_root);
    MenuNode *node = (menu_root) ? menu_root->findUID(uid) : 0;
    if (node) node->clear();
}

//*****************************************************************************
void MenuManager::slotMenuCommand(const char *command)
{
//    debug("MenuManager::slotMenuCommand(%s)", command);    // ###
    emit sigMenuCommand(command);
}

//*****************************************************************************
void MenuManager::addNumberedMenuEntry(const char *uid, char *entry)
{
    ASSERT(menu_root);
    MenuNode *node = (menu_root) ? menu_root->findUID(uid) : 0;
    if (node) {
	const char *cmd = node->getCommand();
	int len = strlen(cmd) + strlen(entry) + 1;
	char *command = new char[len];
	if (!command) {
	    warning("unable to add entry '%s', out of memory ?", entry);
	    return ;
	}
	snprintf(command, len, cmd, entry);
	node->insertLeaf(entry, command, 0, 0, -1);
	delete[] command;
    } else debug ("MenuManager: could not find numbered Menu '%s'", uid);
}

//*****************************************************************************
void MenuManager::selectItem(const char *group, const char *uid)
{
    ASSERT(menu_root);

    if (!group || !*group) {
	warning("MenuManager::selectItem('','%s'): no group!?", uid);
	return ;
    }

    if (*group != '@') {
	warning("MenuManager::selectItem('%s','%s'): "\
		"invalid group name, does not start with '@'!", group, uid);
	return ;
    }

    MenuNode *node = (menu_root) ? menu_root->findUID(group) : 0;
    if (!node) {
	warning("MenuManager::selectItem(): group '%s' not found!", group);
	return ;
    }

    if (!node->inherits("MenuGroup")) {
	warning("MenuManager::selectItem(): '%s' is not a group!", group);
	return ;
    }

    ((MenuGroup *)node)->selectItem(uid);
}

//*****************************************************************************
void MenuManager::setItemChecked(const char *uid, bool check)
{
    ASSERT(menu_root);

//    debug("MenuManager::setItemChecked('%s', %d)", uid, check);
    MenuNode *node = (menu_root) ? menu_root->findUID(uid) : 0;
    if (node) node->setChecked(check);
}

//*****************************************************************************
void MenuManager::setItemEnabled(const char *uid, bool enable)
{
    ASSERT(menu_root);

//    debug("MenuManager::setItemEnabled('%s', %d)", uid, enable);
    MenuNode *node = (menu_root) ? menu_root->findUID(uid) : 0;
    if (node) node->setEnabled(enable);
    else warning("MenuManager::setItemEnabled('%s', '%d'): uid not found!",
		     uid, enable);
}

//*****************************************************************************
MenuManager::~MenuManager()
{
    ASSERT(menu_root);
    if (menu_root) delete menu_root;
}

/* end of MenuManager.cpp */
