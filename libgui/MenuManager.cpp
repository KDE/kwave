
#include <stdio.h>
#include <stdlib.h>

#include <qkeycode.h>

#include <kapp.h>
#include <klocale.h>

#include <libkwave/Parser.h>
#include <libkwave/String.h>

#include "MenuNode.h"
#include "MenuRoot.h"
#include "MenuManager.h"

//*****************************************************************************
MenuManager::MenuManager (QWidget *parent, KMenuBar &bar)
  :QObject(parent)
{
    menu_root = new MenuRoot(bar);
}

//*****************************************************************************
int MenuManager::parseToKeyCode(const char *key_name)
{
    char *key = duplicateString(key_name);
    int keycode=0;
    int len;
    int cnt=0;

    len=strlen(key);
    if (len==1) {
	keycode=Key_A+key[0]-'A';
	delete key;
	return keycode;
    }

    while (cnt<len) {
	int pos=cnt;
	while ((key[cnt])&&(key[cnt]!='+')) cnt++;		
	if (cnt<len) key[cnt]=0;
	
	// keys [A...Z]
	if (strlen(&key[pos])==1) {
	    if ((key[pos]>='A')&&(key[pos]<='Z'))
		keycode+=Key_A+key[pos]-'A';
	}
	
	// function keys [F1...F35] ?
	if (key[pos] == 'F') {
	    int nr = atoi(key+pos+1);
	    if ((nr >= 1) && (nr <= 35)) {
		keycode+=Key_F1+nr-1;
	    }
	}

	// other known keys
	if (!strcmp (&key[pos],"PLUS")) keycode+=Key_Plus;
	if (!strcmp (&key[pos],"MINUS")) keycode+=Key_Minus;
	if (!strcmp (&key[pos],"SPACE")) keycode+=Key_Space;
	if (!strcmp (&key[pos],"CTRL")) keycode+=CTRL;
	if (!strcmp (&key[pos],"PAGEUP")) keycode+=Key_PageUp;
	if (!strcmp (&key[pos],"PAGEDOWN")) keycode+=Key_PageDown;
	if (!strcmp (&key[pos],"UP")) keycode+=Key_Up;
	if (!strcmp (&key[pos],"DEL")) keycode+=Key_Delete;
	if (!strcmp (&key[pos],"DOWN")) keycode+=Key_Down;
	if (!strcmp (&key[pos],"LEFT")) keycode+=Key_Left;
	if (!strcmp (&key[pos],"RIGHT")) keycode+=Key_Right;
	if (!strcmp (&key[pos],"SHIFT")) keycode+=SHIFT;

	cnt++;
    }

    delete key;
    return keycode;
}

//*****************************************************************************
void MenuManager::setCommand (const char *command)
{
    Parser parser(command);

    const char *tmp;
    char *com=0;
    char *pos=0;
    int   key;   // keyboard shortcut (optional)
    char *id=0;  // string id (optional)

    // --- 1st parameter: command to be sent when selected ---
    tmp=parser.getFirstParam ();
    if (tmp) com=duplicateString (tmp);

    // --- 2nd parameter: position in the menu structure ---
    tmp=parser.getNextParam ();
    if (tmp) pos=duplicateString (tmp);

    // bail out if no menu position is found
    if (!pos) {
	debug ("no position field !\n");
	if (com) delete com;
	return;
    }

    // --- 3rd parameter: bitmask for the key shortcut (optional) ---
    tmp=parser.getNextParam();
    key = (tmp) ? parseToKeyCode(tmp) : 0;

    // --- 4rth parameter: parse the string id of the node (optional) ---
    tmp=parser.getNextParam();
    if (tmp) id=duplicateString(tmp);

    // --- insert the new node into the menu structure ---
    menu_root->insertNode(0, pos, com, key, id);

    if (com) deleteString(com);
    if (pos) deleteString(pos);
    if (id)  deleteString(id);
}

//*****************************************************************************
void MenuManager::clearNumberedMenu (const char *uid)
{
    MenuNode *node = menu_root->findUID(uid);
    if (node) node->clear();
}

//*****************************************************************************
void MenuManager::addNumberedMenuEntry (const char *uid, char *entry)
{
    MenuNode *node = menu_root->findUID(uid);
    if (node) {
	const char *cmd  = node->getCommand();
	char *command = new char[strlen(cmd)+strlen(entry)];
        if (!command) {
	    warning("unable to add entry '%s', out of memory ?");
	    return;
        }
	sprintf(command, cmd, entry);
	node->insertLeaf(entry, command, 0, 0, -1);
	delete command;
    } else debug ("MenuManager: could not find numbered Menu '%s'", uid);
}

//*****************************************************************************
void MenuManager::selectItemChecked(const char *uid)
{
    debug("MenuManager::setItemChecked('%s')", uid);
    MenuNode *node = menu_root->findUID(uid);
    if (node) node->setChecked(true);
}

//*****************************************************************************
void MenuManager::setItemChecked(const char *uid, bool check)
{
    debug("MenuManager::setItemChecked('%s', %d)", uid, check);
    MenuNode *node = menu_root->findUID(uid);
    if (node) node->setChecked(check);
}

//*****************************************************************************
void MenuManager::setItemEnabled(const char *uid, bool enable)
{
    // debug("MenuManager::setItemEnabled('%s', %d)", uid, enable);
    MenuNode *node = menu_root->findUID(uid);
    if (node) node->setEnabled(enable);
}

//*****************************************************************************
MenuManager::~MenuManager ()
{
    delete menu_root;
}

/* end of MenuManager.cpp */
