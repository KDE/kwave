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

#include <qdict.h>

#include <kapp.h>

#include <libkwave/Parser.h>
#include <libkwave/String.h>
#include <libkwave/Global.h>
#include <libkwave/MessagePort.h>

#include "MenuNode.h"
#include "MenuGroup.h"

/** globals of kwave, needed for message queue */
extern struct Global globals;

/** unique id for menus */
static int unique_menu_id=0;

/** list of menu nodes */
static QDict<MenuGroup> group_list;

MenuNode::MenuNode(MenuNode *parent, char *name, char *command,
                   int key, char *uid)
    :QObject()
{
    this->parentNode = parent;
    this->name = duplicateString(name);
    this->command = duplicateString(command);
    this->key = key;
    this->icon = 0;
    this->uid = duplicateString(uid);
    this->enabled = true;
    this->last_enabled = true;
    this->checked = false;

    this->id = -1;

    groups.clear();
    children.setAutoDelete(false);
}

//*****************************************************************************
MenuNode::~MenuNode ()
{
    clear();
    deleteString(name);
    deleteString(command);
}

//*****************************************************************************
void MenuNode::actionSelected()
{
    if (command) globals.port->putMessage(command);
}

//*****************************************************************************
void MenuNode::actionChildEnableChanged(int id, bool enable)
{
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
	removeChild(child);
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
MenuNode *MenuNode::getRootNode()
{
    return (parentNode) ? parentNode->getParentNode() : this;
}

//*****************************************************************************
const QPixmap *MenuNode::getIcon()
{
    return icon;
}

//*****************************************************************************
void MenuNode::setIcon(const QPixmap &icon)
{
    this->icon = &icon;
    if (parentNode) parentNode->setItemIcon(id, icon);
    else warning("MenuNode(%s)->parentNode == NULL!!!", getName());
}

//*****************************************************************************
void MenuNode::setItemIcon(int id, const QPixmap &icon)
{
    debug("MenuNode(%s)::setItemIcon(%d, %p)", getName(), id, &icon);
}

//*****************************************************************************
bool MenuNode::isEnabled()
{
    // evaluate our own (individual) enable and our parent's enable state
    if (!enabled) return false;
    if ((parentNode!=0) && !parentNode->isEnabled()) return false;

    // find  out if all our groups are anabled
    MenuNode *root = getRootNode();
    if (root) {
	const char *group_name = groups.first();
	while (group_name) {
	    int pos = groups.at();
	    MenuNode *group = root->findUID(group_name);
	    if (group && group->inherits("MenuGroup")) {
		if (!((MenuGroup*)group)->isEnabled()) {
		    debug("MenuNode(%s).isEnabled(): group %s is disabled",
			getName(), group_name);
		    return false;
		}
	    }
	    groups.at(pos);
	    group_name = groups.next();
	}
    }

    // if we get here, everything is enabled
    return true;
}

//*****************************************************************************
void MenuNode::setEnabled(bool enable)
{
    bool new_enable;

    // store our own individual enable flag
    enabled = enable;

    new_enable = isEnabled(); // get the current effictive state

    if (new_enable != last_enabled) { // on changes:
	last_enabled=new_enable;
	
	 // notify our parent that our enabled state has changed
	emit sigChildEnableChanged(id, new_enable);

	// notify all child nodes that our enable has changed
	emit sigParentEnableChanged();
    }
}

//*****************************************************************************
void MenuNode::slotChildEnableChanged(int id, bool enable)
{
    actionChildEnableChanged(id, enable);
}

//*****************************************************************************
void MenuNode::slotParentEnableChanged()
{
    setEnabled(enabled);
}

//*****************************************************************************
bool MenuNode::isChecked()
{
    return checked;
}

//*****************************************************************************
void MenuNode::setItemChecked(int id, bool check)
{
    return;
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
    unique_menu_id += node->getNeededIDs();

    children.append(node);
    node->setId(new_id);

    // notification for the childs that our enable state changed
    QObject::connect(
	this, SIGNAL(sigParentEnableChanged()),
	node, SLOT(slotParentEnableChanged())    	
    );

    // notification for us that a child's enable state changed
    QObject::connect(
	node, SIGNAL(sigChildEnableChanged(int,bool)),
	this, SLOT(slotChildEnableChanged(int,bool))    	
    );

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
	int pos=children.at();
	MenuNode *node = child->findUID(uid);
	if (node) return node; // found in child
	children.at(pos);
	child = children.next();
    }

    return 0; // nothing found :-(
}

//*****************************************************************************
MenuNode *MenuNode::findChild(char *name)
{
    MenuNode *child = children.first();
    while (child) {
	int pos=children.at();
	if (strcmp(child->getName(),name)==0)
	    return child;
	children.at(pos);
	child = children.next();
    }
    return 0;
}

//*****************************************************************************
MenuNode *MenuNode::findChild(int id)
{
    MenuNode *child = children.first();
    while (child) {
	int pos=children.at();
	if (child->getId() == id)
	    return child;
	children.at(pos);
	child = children.next();
    }
    return 0;
}

//*****************************************************************************
void MenuNode::removeChild(MenuNode *child)
{
    if (!child) return;

    int index = children.find(child);
    if (index != -1) {
	children.remove(index);
    }

    // notification for the childs that our enable state changed
    QObject::disconnect(
	this, SIGNAL(sigParentEnableChanged()),
	child, SLOT(slotParentEnableChanged())    	
    );

    // notification for us that a child's enable state changed
    QObject::disconnect(
	child, SIGNAL(sigChildEnableChanged(int,bool)),
	this, SLOT(slotChildEnableChanged(int,bool))    	
    );
}

//*****************************************************************************
MenuNode *MenuNode::insertBranch(char *name, char *command, int key,
                                 char *uid, int index)
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
    if (*position == '/') *(position++)=0;

    if ((name) && (specialCommand(name))) {
	// no new branch, only a special command
	return 0;
    }

    if ((*position == 0) || (*position == '#')) {
	// end of the tree
	MenuNode *sub = findChild(name);
	if (sub) {
	    // a leaf with this name already exists
	    // -> maybe we want to set new properties
	    if (key) sub->setKey(key);
	    if (strlen(uid)) sub->setUID(uid);
	
	    if (*position == '#') sub->specialCommand(position);
	    return sub->getId();
	} else {
	    // insert a new leaf
	    MenuNode *leaf = insertLeaf(name, command, key, uid);
	    if (!leaf) return -1;
	
	    if (*position == '#') leaf->specialCommand(position);
	    return leaf->getId();
	}
    } else {
    	// somewhere in the tree
	MenuNode *sub = findChild(name);
	if (!sub) {
	    sub = insertBranch(name, command, key, uid);
	} else if ( !sub->isBranch() && (*position != '#')) {
	    // remove the "leaf" and insert a branch with
	    // the same properties
	    sub = leafToBranch(sub);
	} else if ( (*position == '#') || (*position == 0) ) {
	    // branch already exists and we are at the end of parsing
	    // -> maybe we want to set new properties
	    if (key) sub->setKey(key);
	    if (strlen(uid)) sub->setUID(uid);
	}
	
	if (sub) {
	    result = sub->insertNode(0, position, command, key, uid);
	} else {
	    debug("MenuNode::insertNode: branch failed!");
	}
    }

    return result;
}

//*****************************************************************************
MenuNode *MenuNode::leafToBranch(MenuNode *node)
{
    // debug("MenuNode::leafToBranch(%s)", node->getName());
    MenuNode *sub = node;

    // get the old properties
    int index                     = sub->getIndex();
    int old_key                   = sub->getKey();
    char *old_uid                 = sub->getUID();
    const QPixmap *old_icon       = sub->getIcon();
    char *name                    = duplicateString(node->getName());
    char *command                 = duplicateString(node->getCommand());
    QList<const char> &old_groups = sub->groups;

    // remove the old child node
    removeChild(sub);

    // insert the new branch
    sub = insertBranch(name, command, old_key, old_uid, index);
    if (sub) {
	// join it to the same groups
	const char *group = old_groups.first();
	while (group) {
	    int pos=old_groups.at();
	    sub->joinGroup(group);
	    old_groups.at(pos);
	    group = old_groups.next();
	}

	// set the old icon
	if (old_icon) sub->setIcon(*old_icon);
    }

    delete name;
    delete command;

    return sub;
}

//*****************************************************************************
void MenuNode::joinGroup(const char *group)
{
    if (groups.find(group) != -1) return; // already joined

    const char *group_name = duplicateString(group);
    MenuGroup *grp = group_list.find(group);
    if (!grp) {
	// group does not already exist, create a new one
	grp = new MenuGroup(getRootNode(), (char *)group_name);
        if (grp) group_list.insert(group_name, grp);
    }

    // remind that we belong to the given group
    groups.append(group_name);

    // register this node as a child of the group
    if (grp) grp->registerChild(this);
}

//*****************************************************************************
void MenuNode::leaveGroup(const char *group)
{
    if (groups.find(group) == -1) return; // not joined

    MenuGroup *grp = group_list.find(group);
    if (!grp) return; // group does not exist

    // remove our child node from the group
    grp->removeChild(this);

    // remove the group from our list
    groups.remove(group);
}

//*****************************************************************************
bool MenuNode::specialCommand(const char *command)
{
    if (strncmp(command,"#group(",7) == 0) {
	Parser parser(command);
	const char *group;
	
	// join to a list of groups
	group = parser.getFirstParam();
	while (group) {
	    joinGroup(group);
	    group = parser.getNextParam();
	}
	return true;
    }
    return false;
}
	
/* end of MenuNode.cpp */