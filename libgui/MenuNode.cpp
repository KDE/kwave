/***************************************************************************
                          MenuNode.cpp  -  description
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

#include <stdio.h>
#include <libkwave/Parser.h>
#include <libkwave/String.h>
#include <libkwave/Global.h>
#include <libkwave/MessagePort.h>
#include <kapp.h>
#include "MenuCommand.h"
#include "MenuNode.h"

extern struct Global globals;

static int unique_menu_id=0;

MenuNode::MenuNode(MenuNode *parent, char *name, char *command,
                   int key, char *uid)
    :QObject()
{
    this->parentNode = parent;
    this->name = duplicateString (name);
    this->command = duplicateString(command);
    this->key = key;
    this->uid = duplicateString(uid);
    this->enabled = true;
    this->checked = false;

    this->id = -1;
    children.setAutoDelete(true);
}

//*****************************************************************************
MenuNode::~MenuNode ()
{
    children.setAutoDelete(true);
    children.clear();
    deleteString(name);
    deleteString(command);
}

//*****************************************************************************
void MenuNode::actionSelected()
{
    /*
  if (numberItems)
    {
      char buf[512];
      char *tmp=duplicateString (this->com);
      int cnt=strlen (tmp);
      while ((cnt)&&(tmp[cnt]!=')')) cnt--;
      tmp[cnt]=0;
      sprintf (buf,"%d",num);

      char *com=catString (tmp,buf,")");

      globals.port->putMessage (com);

      deleteString (com);
      deleteString (tmp);
    }
    */
    if (command) globals.port->putMessage(command);
}

//*****************************************************************************
/*
void MenuNode::slotHilighted(int id)
{
  // (this is useful for debugging menu ids)
  debug("MenuNode::hilight(%d)", id);
  MenuNode *parent = getParent();
  setCheckable(true);
  if (parent) {
      for (int i=0; i < 300; i++) {
          parent->setItemChecked(i, false);
      }
      parent->setItemChecked(id, true);
  }
}
*/

//*****************************************************************************
void MenuNode::clear()
{
    // clear all children
    MenuNode *child;
    while ( (child=children.first()) != 0 ) {
	child->clear();
	children.remove(child);
    }
}

//*****************************************************************************
int MenuNode::getChildIndex(int id)
{
    return -1;
}

//*****************************************************************************
MenuNode *MenuNode::getParentNode()
{
    return parentNode;
}

//*****************************************************************************
bool MenuNode::isEnabled()
{
    return enabled;
}

//*****************************************************************************
bool MenuNode::setItemEnabled(int id, bool enable)
{
    return false;
}

//*****************************************************************************
void MenuNode::setEnabled(bool enable)
{
    debug("MenuNode(%s)::setEnabled(%d)", getName(), enable);
    enabled = enable;
}

//*****************************************************************************
bool MenuNode::isChecked()
{
    return checked;
}

//*****************************************************************************
bool MenuNode::setItemChecked(int id, bool check)
{
    if (id == getId()) {
	// check/uncheck myself
	return (parentNode) ?
	    parentNode->setItemChecked(getId(), check) :
	    false;
    } else {
	// go through all children
	MenuNode *child = children.first();
	while (child) {
	    if (child->setItemChecked(id, check))
		return true;
	    child = children.next();
	}
    }
    return false;
}

//*****************************************************************************
void MenuNode::setChecked(bool check)
{
    checked = check;
}

//*****************************************************************************
int MenuNode::getNeededIDs()
{
    return 1;
}

//*****************************************************************************
int MenuNode::registerChild(MenuNode *node)
{
    int new_id;
    if (!node) return -1;

    new_id = unique_menu_id;
    unique_menu_id += getNeededIDs();

    children.append(node);
    node->setId(new_id);

    return new_id;
}

//*****************************************************************************
void MenuNode::setUID(char *uid)
{
    if (this->uid) delete(this->uid);
    this->uid = duplicateString(uid);
}

//*****************************************************************************
MenuNode *MenuNode::findUID(const char *uid)
{
    if (strcmp(uid, this->uid) == 0) return this; // found ourself

    MenuNode *child = children.first();
    while (child) {
	MenuNode *node = child->findUID(uid);
	if (node) return node; // found in child
	child = children.next();
    }

    return 0; // nothing found :-(
}

//*****************************************************************************
MenuNode *MenuNode::findChild(char *name)
{
    MenuNode *child = children.first();
    while (child) {
	if (strcmp(child->getName(),name)==0)
	    return child;
	child = children.next();
    }
    return 0;
}

//*****************************************************************************
MenuNode *MenuNode::findChild(int id)
{
    MenuNode *child = children.first();
    while (child) {
	if (child->getId() == id)
	    return child;
	child = children.next();
    }
    return 0;
}

//*****************************************************************************
void MenuNode::removeChild(int id)
{
    MenuNode *child = findChild(id);
    if (child) {
	int index = children.find(child);
	if (index != -1) {
	    children.remove(index);
	}
    }
}

//*****************************************************************************
MenuNode *MenuNode::insertBranch(char *name, int key, char *uid, int index)
{
    debug("!!! MenuNode(%s): insertBranch(%s) !!!", this->name, name);
    return 0;
}

//*****************************************************************************
MenuNode *MenuNode::insertLeaf(char *name, char *command, int key, char *uid,
                               int index)
{
    debug("!!! MenuNode(%s): insertLeaf(%s) !!!", this->name, name);
    return 0;
}

//*****************************************************************************
int MenuNode::insertNode(char *name, char *position, char *command,
                         int key, char *uid)
{
    int result = -1;

    if ((position == 0) || (*position == 0)) {
	debug("MenuNode::parseCommand: no position!"); // ###
	return result;
    }

    // at start of the parsing process ?
    if ((name == 0) || (*name == 0)) {
	// split off the first token, separated by a slash
	name = position;
	while ((*position) && (*position != '/'))
	    position++;
    }
    if ((position) && (*position == '/'))
	*(position++)=0;

    // --- parse special commands first ---
    if (specialCommand(name)) return 0;

    if ((position == 0) || (*position == 0)) {
	// end of the tree
	MenuNode *sub = findChild(name);
	if (sub) {
	    // a leaf with this name already exists
	    if (key) sub->setKey(key);
	    if (strlen(uid)) sub->setUID(uid);
	    return sub->getId();
	} else {
	    // insert a new leaf
	    MenuNode *leaf = insertLeaf(name, command, key, uid);
	    return (leaf) ? leaf->getId() : -1;
	}
    } else {
    	// somewhere in the tree
    	MenuNode *sub;

    	sub = findChild(name);
	if (!sub) {
	    sub = insertBranch(name, key, uid);
	} else if (! sub->isBranch()) {
	    // remove the "leaf" and insert a branch with
	    // the same properties
	    int index     = sub->getIndex();
	    int old_key   = sub->getKey();
	    char *old_uid = sub->getUID();
	    int old_id    = sub->getId();
	
	    removeChild(old_id);
	    sub = insertBranch(name, old_key, old_uid, index);

	} else {
	    // branch already exists (maybe we want to set new properties)
	    if (key) sub->setKey(key);
	    if (strlen(uid)) sub->setUID(uid);
	}
	
	if (sub) {
	    result = sub->insertNode(0, position, command, key, uid);
	} else {
	    debug("MenuNode::insertNode: branch failed!"); // ###
	}
    }

//    debug("MenuNode::insertNode: returning with %d", result);
    return result;
}

//*****************************************************************************
bool MenuNode::specialCommand(const char *command)
{
    return false;
}

	
/* end of MenuNode.cpp */