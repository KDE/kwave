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

#include "config.h"
#include <stdio.h>
#include <qdict.h>
#include <kapp.h>
#include <libkwave/Parser.h>
#include <libkwave/String.h>
#include "MenuNode.h"
#include "MenuGroup.h"

/** unique id for menus */
static int unique_menu_id = 0;

//static int menu_node_count = 0;

MenuNode::MenuNode(MenuNode *parent, char *name, char *command,
		   int key, char *uid)
    :QObject(),
    groups(false)
{
//    menu_node_count++;
//    debug("MenuNode::MenuNode(): node count=%d", menu_node_count);

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

    ASSERT(name);

    groups.clear();
    children.setAutoDelete(false);
}

//*****************************************************************************
MenuNode::~MenuNode()
{
//    debug("MenuNode(%s)::~MenuNode()", getName());

    // deregister from our parent
    if (parentNode) parentNode->removeChild(this);

    // leave all groups
    const char *group = groups.first();
    while (group) {
	leaveGroup(group);
	group = groups.first();
    }

    clear();

//    debug("MenuNode(%s)::~MenuNode(): done.", getName());
    if (name) delete[] name;
    if (command) delete[] command;
    if (uid) delete[] uid;

//    menu_node_count--;
//    debug("MenuNode::MenuNode(): node count=%d", menu_node_count);
}

//*****************************************************************************
void MenuNode::emitCommand(const char *command)
{
    if (!command) return ;

    if (!getParentNode()) {
	// no parent -> we are the root node -> we have to emit
	emit sigCommand(command);
    } else {
	// tell the root node to emit
	MenuNode *root = getRootNode();
	ASSERT(root);
	if (root) root->emitCommand(command);
    }
}

//*****************************************************************************
void MenuNode::actionSelected()
{
    if (command) emitCommand(command);
}

//*****************************************************************************
void MenuNode::actionChildEnableChanged(int id, bool enable) {}

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
    MenuNode *child;

//    debug("------- start -----------");
//    child = children.first();
//    while (child) {
//	debug("MenuNode(%s)::clear: %s : %p, id=%d", getName(),
//	    child->getName(),child,child->getId());
//       child = children.next();
//    }
//    debug("------- end -----------");

    // clear all children
    child = children.first();
    while (child) {
	removeChild(child);
	delete child;
	child = children.first();
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
    return (parentNode) ? parentNode->getRootNode() : this;
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
    if ((parentNode != 0) && !parentNode->isEnabled()) return false;

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

    new_enable = isEnabled();    // get the current effictive state

    if (new_enable != last_enabled) { // on changes:
	last_enabled = new_enable;

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
    return ;
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
    ASSERT(node);
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
	node, SIGNAL(sigChildEnableChanged(int, bool)),
	this, SLOT(slotChildEnableChanged(int, bool))
    );

    return new_id;
}

//*****************************************************************************
void MenuNode::setUID(char *uid)
{
    if (this->uid) delete(this->uid);
    this->uid = (uid) ? duplicateString(uid) : 0;
}

//*****************************************************************************
MenuNode *MenuNode::findUID(const char *uid)
{
    if (strcmp(uid, this->uid) == 0) return this;    // found ourself

    MenuNode *child = children.first();
    while (child) {
	int pos = children.at();
	MenuNode *node = child->findUID(uid);
	if (node) return node;    // found in child
	children.at(pos);
	child = children.next();
    }

    return 0;    // nothing found :-(
}

//*****************************************************************************
MenuNode *MenuNode::findChild(char *name)
{
    ASSERT(name);
    MenuNode *child = children.first();
    while (child) {
	int pos = children.at();
	if (strcmp(child->getName(), name) == 0)
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
	int pos = children.at();
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
    ASSERT(child);
    if (!child) return ;

    // notification for the childs that our enable state changed
    QObject::disconnect(
	this, SIGNAL(sigParentEnableChanged()),
	child, SLOT(slotParentEnableChanged())
    );

    // notification for us that a child's enable state changed
    QObject::disconnect(
	child, SIGNAL(sigChildEnableChanged(int, bool)),
	this, SLOT(slotChildEnableChanged(int, bool))
    );

    children.setAutoDelete(false);
    children.remove(child);
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
	debug("MenuNode::parseCommand: no position!");    // ###
	return result;
    }

    // at start of the parsing process ?
    if ((name == 0) || (*name == 0)) {
	// split off the first token, separated by a slash
	name = position;
	while ((*position) && (*position != '/'))
	    position++;
    }
    if (*position == '/') *(position++) = 0;

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
    ASSERT(node);
//    debug("MenuNode::leafToBranch(%s)", node->getName());
    if (!node) return 0;
    MenuNode *sub = node;

    // get the old properties
    int index = sub->getIndex();
    int old_key = sub->getKey();
    char *old_uid = sub->getUID();
    const QPixmap *old_icon = sub->getIcon();
    char *name = duplicateString(node->getName());
    char *command = duplicateString(node->getCommand());
    QStrList &old_groups = sub->groups;

    // remove the old child node
    removeChild(sub);

    // insert the new branch
    sub = insertBranch(name, command, old_key, old_uid, index);
    if (sub) {
	// join it to the same groups
	const char *group = old_groups.first();
	while (group) {
	    int pos = old_groups.at();
	    sub->joinGroup(group);
	    old_groups.at(pos);
	    group = old_groups.next();
	}

	// set the old icon
	if (old_icon) sub->setIcon(*old_icon);
    }

    delete node;    // free the old node
    delete[] name;
    delete[] command;

    return sub;
}

//*****************************************************************************
QDict<MenuNode> *MenuNode::getGroupList()
{
    ASSERT(parentNode);
    return (parentNode) ? parentNode->getGroupList() : 0;
}

//*****************************************************************************
void MenuNode::joinGroup(const char *group)
{
    ASSERT(parentNode);
    QDict<MenuNode> *group_list = getGroupList();
    if (groups.find(group) != -1) return ;    // already joined

    MenuGroup *grp = (group_list) ? (MenuGroup *)group_list->find(group) : 0;
    if (!grp) {
	// group does not already exist, create a new one
	grp = new MenuGroup(getRootNode(), (char *)group);
	if (grp) group_list->insert(group, grp);
    }

    // remind that we belong to the given group
    groups.append(duplicateString(group));

    // register this node as a child of the group
    if (grp) grp->registerChild(this);
}

//*****************************************************************************
void MenuNode::leaveGroup(const char *group)
{
    QDict<MenuNode> *group_list = getGroupList();
    int pos = groups.find(group);
    MenuGroup *grp = (group_list) ? (MenuGroup *)group_list->find(group) : 0;

    // remove the group from our list
    groups.setAutoDelete(true);
    if (pos != -1) groups.remove(pos);

    // remove ourself from the group
    if (grp) grp->removeChild(this);
}

//*****************************************************************************
bool MenuNode::specialCommand(const char *command)
{
    if (strncmp(command, "#group(", 7) == 0) {
	Parser parser(command);
	const char *group;

	// join to a list of groups
	group = parser.getFirstParam();
	while (group) {
	    joinGroup(group);
	    group = parser.getNextParam();
	}
	return true;
    } else if (strncmp(command, "#disable", 8) == 0) {
	// disable the node
	setEnabled(false);
	return true;
    } else if (strncmp(command, "#enable", 7) == 0) {
	// disable the node
	setEnabled(true);
	return true;
    }
    return false;
}

/* end of MenuNode.cpp */