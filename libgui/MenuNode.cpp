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

/* ###
int MenuNode::getUniqueId ()
{
  int x=unique_menu_id;
  unique_menu_id++;

  return x;
}

int MenuNode::getIdRange (int num)
{
  debug("MenuNode::getIdRange(%d", num);
  int x=unique_menu_id;
  unique_menu_id+=num;

  return x;
}
*/

MenuNode::MenuNode (const char *name)
    :QObject()
{
  this->parentNode=0;
  this->name=duplicateString (name);
  this->id=-1;
  children.setAutoDelete(true);
/* ###
  this->toplevel=false; // ### toplevel;
  this->toplevelEnabled=true;
  this->enabled=true;
  QObject::connect (this,SIGNAL(activated(int)),this,SLOT(selected(int)));
  QObject::connect (this,SIGNAL(highlighted(int)),this,SLOT(hilight(int)));
  checkItems=false;
  numberItems=false;
  com=0;
### */
}
/*
void MenuNode::hilighted(int item)
{
    debug("%s:hilighted:%d", getName(), item);
}
*/
/* ###
void MenuNode::setCommand (const char *com)
  //sets the command emitted when selecting the menu entry
{
  this->com=duplicateString (com);
}

void MenuNode::setNumberable ()
{
  numberItems=true;
}

void MenuNode::setCheckable ()
  //enables checking of menu entries
{
  checkItems=true;
  QObject::connect (this,SIGNAL(activated(int)),this,SLOT(check(int)));
  QPopupMenu::setCheckable (true);
  checked=-1;
}
### */

void MenuNode::selected (int num)
{
/* ###
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
  else
    {
      MenuCommand *tmp=commands.first();

      while (tmp)
	{
	  if (tmp->getId()==num)
	    globals.port->putMessage (tmp->getCommand());

	  tmp=commands.next();
	}
    }
    ### */
}
/* ###
void MenuNode::setEnabled (bool enable)
{
  this->enabled=enable;

//  debug("MenuNode::setEnabled(), menu='%s', id=%d", getName(), getId());

  MenuNode *parentMenu = this->getParentNode();
  if (!this->isTopLevel() && parentMenu)
    {
      parentMenu->setItemEnabled(this->getId(),
	(enable && toplevelEnabled));
    }
  else
    {
      MenuNode *child=children.first();
      while (child)
	{
	  child->setTopLevelEnabled(enable);
	  child=children.next();
	}
    }
}
*/

/**
 * Sets/resets an internal flag of the menu that indicates that it is a
 * toplevel menu. This setting affects the behaviour of the setEnabled()
 * function.
 * <p>
 * @param enable true=toplevel, false if not
 * @see MenuNode::setEnabled(bool)
 */
/* ###void MenuNode::setTopLevelEnabled (bool enable)
{
  toplevelEnabled=enable;
  setEnabled(this->enabled);
}
*/

//*****************************************************************************
/**
 * Checks a menu item. The checkmark of a currently checked menu item
 * will be removed, so that this method could be used for 1-of-n
 * selections. The currently selected item id will be set to the new
 * id.
 * <p>
 * @param id unique id of the menu or menu entry
 */
/* ###void MenuNode::checkEntry(int id)
{
//  if (! checkItems) return;

  debug("MenuNode::checkEntry(%d): checked=%d", id, checked);
  debug("(this=%p, name='%s', id=%d)", this,
    this->getName(), this->getId());

  if (checked >= 0) this->checkEntry(checked, false);
  checked = id;
  this->checkEntry(id, true);
}
### */

/**
 * Checks or unchecks a menu item. The checkmark of a currently
 * checked menu item will not be removed, so that this method
 * could be used for multiple-choice selections. The currently selected
 * item id will not be changed.
 * <p>
 * @param id unique id of the menu or menu entry
 * @param checked true=checkmark on, false=checkmark off
 */
/* ###
void MenuNode::checkEntry(int id, bool check)
{
  debug("MenuNode::checkEntry(%d,%d): checked=%d",
    id, check, checked);
  setItemChecked(id, check);
}
### */

void MenuNode::check(int id)
{
/**   MenuNode *parent = getParent();
  if (parent) parent->
  checkEntry(id); */
}

void MenuNode::hilight(int id)
{
 /* ###
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
  */
}

/* ###
int MenuNode::insertEntry (const char *name,const char *command, int keycode, int id)
{
  int key;
  MenuNode *dummy;

  dummy = new MenuNode(name, id);
  commands.append (new MenuCommand (command,id));
  children.append (dummy);

  key=this->insertItem (klocale->translate(name),id);
  this->setAccel (keycode,id);

  dummy->setId(key);
  dummy->setParent(this);
  return key; // return the real id of the menu entry
### }
*/

/*
int MenuNode::insertMenu (MenuNode *entry)
{
  int key;
  children.append (entry);
  key = this->insertItem(entry->getName(), entry, entry->getId());
  entry->setId(key);
  entry->setParent(this);
  return key;
}

void MenuNode::removeMenu (const char *name)
{
  MenuNode *menu=findMenu(name);
  if (menu)
    {
      int index=menu->getId();
      removeItem(index);
      children.removeRef (menu);
      delete menu;
    }
}

MenuNode *MenuNode::findMenu (const char *name)
{
    MenuNode *tmp=children.first();
    while (tmp) {
	if (strcmp (tmp->getName(),name)==0) return tmp;
	tmp=children.next();
    }
    return 0;
}
*/

//*****************************************************************************
MenuNode::~MenuNode ()
{
    deleteString (name);
// ###    deleteString (com);
}

/** removes all entries */
void MenuNode::clear()
{
    // clear all children
    for (MenuNode *child=children.first(); (child); child=children.next())
	child->clear();
}

int MenuNode::getChildIndex(const int id)
{
    return -1;
}

/** returns a pointer to the menu's parent node */
MenuNode * MenuNode::getParentNode()
{
    return parentNode;
}

/** sets a new pointer to the node's parent */
void MenuNode::setParent(MenuNode *newParent)
{
    parentNode = newParent;
}

/**
 * Enables or disables an item of a menu. If the specified item is not
 * a member of the current menu, this method will recursively call all
 * of it's child nodes.
 * @param id the unique menu item id
 * @param enable true to enable the item, false to disable
 * @return true if the item has been found, false if not
 */
bool MenuNode::setItemEnabled(const int id, const bool enable)
{
    if (id == getId()) {
	// enable/disable myself
	return (parentNode) ?
	    parentNode->setItemEnabled(getId(), enable) :
	    false;
    } else {
	// go through all children
	MenuNode *child = children.first();
	while (child) {
	    if (child->setItemEnabled(id, enable))
		return true;
	    child = children.next();
	}
    }
    return false;
}

/**
 * Returns the number of ids a menu node needs.
 */
int MenuNode::getNeededIDs()
{
    return 1;
}

/**
 * Inserts a new child node into the current structure.
 * @param node pointer of the new node
 * @return the node's unique id or -1 if the node is null
 */
int MenuNode::registerChild(MenuNode *node)
{
    int new_id;
    if (!node) return -1;

    new_id = unique_menu_id;
    unique_menu_id += getNeededIDs();

    children.append(node);
    node->setParent(this);
    node->setId(new_id);

    return new_id;
}

/**
 * Tries to find a child node by it's name and returns a pointer
 * to it. If the child can't be found the return value will be 0.
 * @param name the item's name (non-localized)
 * @return pointer to the found child or 0
 */
MenuNode *MenuNode::findChild(const char *name)
{
    MenuNode *child = children.first();
    while (child) {
	if (strcmp(child->getName(),name)==0)
	    return child;
	child = children.next();
    }
    return 0;
}

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

void MenuNode::removeChild(const int id)
{
    MenuNode *child = findChild(id);
    if (child) {
	int index = children.find(child);
	if (index != -1) {
	    children.remove(index);
	}
    }
}

MenuNode *MenuNode::insertBranch(char *name, const int key, const char *uid,
	                         const int index)
{
    debug("!!! MenuNode(%s): insertBranch(%s) !!!", this->name, name);
    return 0;
}

MenuNode *MenuNode::insertLeaf(const char *command, char *name,
                               const int key, const char *uid,
                               const int index)
{
    debug("!!! MenuNode(%s): insertLeaf(%s) !!!", this->name, name);
    return 0;
}

int MenuNode::insertNode(const char *command, char *name, char *position,
                         const int key, const char *uid)
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
    if ((*position) && (*position == '/'))
	*(position++)=0;

//    debug("MenuNode(%s)::insertNode('%s','%s')", getName(), name, position);// ###


    if (specialCommand(name)) return 0;

    if ((position == 0) || (*position == 0)) {
	// end of the tree
	MenuNode *sub = findChild(name);
	if (sub) {
	    // a branch with this name already exists
	    return sub->getId();
	} else {
	    // insert a new leaf
	    MenuNode *leaf = insertLeaf(command, name, key, uid);
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
	    int index = sub->getIndex();
	    int old_id = sub->getId();
	
	    debug("MenuNode:insertNode(%s):removing child %s"\
		" (id=%d, index=%d)", getName(), sub->getName(),
		old_id, index); // ###
	    removeChild(old_id);
	
	    sub = insertBranch(name, key, uid, index);
	}
	
	if (sub) {
	    result = sub->insertNode(command, 0, position, key, uid);
	} else {
	    debug("MenuNode::insertNode: branch failed!"); // ###
	}
    }

//    debug("MenuNode::insertNode: returning with %d", result);
    return result;
}

bool MenuNode::specialCommand(const char *command)
{
    return false;
}

	
/* end of MenuNode.cpp */