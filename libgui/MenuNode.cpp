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

#include "libkwave/Parser.h"

#include "MenuNode.h"
#include "MenuGroup.h"

/** unique id for menus */
static int unique_menu_id = 0;

//static int menu_node_count = 0;

MenuNode::MenuNode(MenuNode *parent, const QString &name,
	const QString &command, int key, const QString &uid)
    :QObject(),
    m_groups(),
    m_icon(0)
{
//    menu_node_count++;
//    debug("MenuNode::MenuNode(): node count=%d", menu_node_count);

    m_parentNode = parent;
    m_name = name;
    m_command = command;
    m_key = key;
    m_uid = uid;
    m_enabled = true;
    m_last_enabled = true;
    m_checked = false;
    m_id = -1;

    m_groups.clear();
    m_children.setAutoDelete(false);
}

//*****************************************************************************
MenuNode::~MenuNode()
{
    // leave all groups
    QString group = m_groups.first();
    while (group) {
	leaveGroup(group);
	group = m_groups.first();
    }

    clear();

    // deregister from our parent
    if (m_parentNode) m_parentNode->removeChild(this);
}

//*****************************************************************************
void MenuNode::emitCommand(const QString &command)
{
    ASSERT(command.length());
    if (!command.length()) return ;

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
    if (m_command.length()) emitCommand(m_command);
}

//*****************************************************************************
void MenuNode::actionChildEnableChanged(int /*id*/, bool /*enable*/)
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
    child = m_children.first();
    while (child) {
	removeChild(child);
	delete child;
	child = m_children.first();
    }
}

//*****************************************************************************
int MenuNode::getChildIndex(int /*id*/)
{
    return -1;
}

//*****************************************************************************
MenuNode *MenuNode::getParentNode()
{
    return m_parentNode;
}

//*****************************************************************************
MenuNode *MenuNode::getRootNode()
{
    return (m_parentNode) ? m_parentNode->getRootNode() : this;
}

//*****************************************************************************
const QPixmap &MenuNode::getIcon()
{
    return m_icon;
}

//*****************************************************************************
void MenuNode::setIcon(const QPixmap icon)
{
    m_icon = icon;
    if (m_parentNode) m_parentNode->setItemIcon(m_id, icon);
}

//*****************************************************************************
void MenuNode::setItemIcon(int id, const QPixmap &icon)
{
    debug("MenuNode(%s)::setItemIcon(%d, %p)", getName().data(), id, &icon);
}

//*****************************************************************************
bool MenuNode::isEnabled()
{
    // evaluate our own (individual) enable and our parent's enable state
    if (!m_enabled) return false;
    if ((m_parentNode != 0) && !m_parentNode->isEnabled()) return false;

    // find  out if all our groups are anabled
    MenuNode *root = getRootNode();
    if (root) {
	QStringList::Iterator it = m_groups.begin();
	for ( ; it != m_groups.end(); ++it) {
	    ASSERT(it != 0);
	    QString group_name = *it;
	    MenuNode *group = root->findUID(group_name);
	    if (group && group->inherits("MenuGroup")) {
		if (!((MenuGroup*)group)->isEnabled()) {
		    debug("MenuNode(%s).isEnabled(): group %s is disabled",
			  getName().data(), group_name.data());
		    return false;
		}
	    }
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
    m_enabled = enable;

    new_enable = isEnabled();    // get the current effictive state

    if (new_enable != m_last_enabled) { // on changes:
	m_last_enabled = new_enable;

	// notify our parent that our enabled state has changed
	emit sigChildEnableChanged(m_id, new_enable);

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
    setEnabled(m_enabled);
}

//*****************************************************************************
bool MenuNode::isChecked()
{
    return m_checked;
}

//*****************************************************************************
void MenuNode::setItemChecked(int /*id*/, bool /*check*/)
{
    return ;
}

//*****************************************************************************
void MenuNode::setChecked(bool check)
{
    m_checked = check;
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

    m_children.append(node);
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
void MenuNode::setUID(const QString &uid)
{
    m_uid = uid;
}

//*****************************************************************************
MenuNode *MenuNode::findUID(const QString &uid)
{
    if (m_uid == uid) return this;    // found ourself

    MenuNode *child = m_children.first();
    while (child) {
	int pos = m_children.at();
	MenuNode *node = child->findUID(uid);
	if (node) return node;    // found in child
	m_children.at(pos);
	child = m_children.next();
    }

    return 0;    // nothing found :-(
}

//*****************************************************************************
MenuNode *MenuNode::findChild(const QString &name)
{
    ASSERT(name);
    MenuNode *child = m_children.first();
    while (child) {
	int pos = m_children.at();
	if (name == child->getName())
	    return child;
	m_children.at(pos);
	child = m_children.next();
    }
    return 0;
}

//*****************************************************************************
MenuNode *MenuNode::findChild(int id)
{
    MenuNode *child = m_children.first();
    while (child) {
	int pos = m_children.at();
	if (child->getId() == id)
	    return child;
	m_children.at(pos);
	child = m_children.next();
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

    m_children.setAutoDelete(false);
    m_children.remove(child);
}

//*****************************************************************************
MenuNode *MenuNode::insertBranch(const QString &name,
	const QString &/*command*/, int /*key*/, const QString &/*uid*/,
	int /*index*/)
{
    debug("!!! MenuNode(%s): insertBranch(%s) !!!", m_name.data(), name.data());
    return 0;
}

//*****************************************************************************
MenuNode *MenuNode::insertLeaf(const QString &name,
	const QString &/*command*/, int /*key*/, const QString &/*uid*/,
	int /*index*/)
{
    debug("!!! MenuNode(%s): insertLeaf(%s) !!!", m_name.data(), name.data());
    return 0;
}

//*****************************************************************************
int MenuNode::insertNode(const QString &name, const QString &position,
                         const QString &command, int key, const QString &uid)
{
    int result = -1;
    int pos = 0;

    if (!position.length()) {
	warning("MenuNode::parseCommand: no position!");
	return result;
    }

    // make working copies of name and position
    QString n(name);
    QString p(position);

    // at start of the parsing process ?
    if (!n.length()) {
	// split off the first token, separated by a slash
	pos = p.find('/');
	if (pos < 0) pos = p.length();
    }
    n = position.left(pos);
    p.remove(0, pos+1);
//
//    debug("MenuNode::parseCommand: ---");    // ###
//    debug("MenuNode::parseCommand:      pos=%d'",pos);    // ###
//    debug("MenuNode::parseCommand:     name='%s'",n.data());    // ###
//    debug("MenuNode::parseCommand: position='%s'",p.data());    // ###
//    debug("MenuNode::parseCommand: ---");    // ###
//
    if ((n.length()) && (specialCommand(n))) {
	// no new branch, only a special command
	return 0;
    }

    if ((!p.length()) || (p[0] == '#')) {
	// end of the tree
	MenuNode *sub = findChild(n);
	if (sub) {
	    // a leaf with this name already exists
	    // -> maybe we want to set new properties
	    if (key) sub->setKey(key);
	
	    if (uid.length()) sub->setUID(uid);

	    if (p[0] == '#') sub->specialCommand(p);
	    return sub->getId();
	} else {
	    // insert a new leaf
	    MenuNode *leaf = insertLeaf(n, command, key, uid);
	    if (!leaf) return -1;

	    if (p[0] == '#') leaf->specialCommand(p);
	    return leaf->getId();
	}
    } else {
	// somewhere in the tree
	MenuNode *sub = findChild(n);
	if (!sub) {
	    sub = insertBranch(n, command, key, uid);
	} else if ( !sub->isBranch() && (p[0] != '#')) {
	    // remove the "leaf" and insert a branch with
	    // the same properties
	    sub = leafToBranch(sub);
	} else if ( (p[0] == '#') || (p[0] == 0) ) {
	    // branch already exists and we are at the end of parsing
	    // -> maybe we want to set new properties
	    if (key) sub->setKey(key);
	    if (uid.length()) sub->setUID(uid);
	}

	if (sub) {
	    result = sub->insertNode(0, p, command, key, uid);
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
    QString old_uid = sub->getUID();
    const QPixmap &old_icon = sub->getIcon();
    QString name = node->getName();
    QString command = node->getCommand();
    QStringList old_groups = sub->m_groups;

    // remove the old child node
    removeChild(sub);

    // insert the new branch
    sub = insertBranch(name, command, old_key, old_uid, index);
    if (sub) {
	// join it to the same groups
	QStringList::Iterator it = old_groups.begin();
	for (; it != old_groups.end(); ++it) {
	    sub->joinGroup(*it);
	}
	
	// set the old icon
	if (!old_icon.isNull()) sub->setIcon(old_icon);
    }

    delete node;    // free the old node

    return sub;
}

//*****************************************************************************
QDict<MenuNode> *MenuNode::getGroupList()
{
    ASSERT(m_parentNode);
    return (m_parentNode) ? m_parentNode->getGroupList() : 0;
}

//*****************************************************************************
void MenuNode::joinGroup(const QString &group)
{
    ASSERT(m_parentNode);
    QDict<MenuNode> *group_list = getGroupList();
    if (m_groups.contains(group)) return ;    // already joined

    MenuGroup *grp = (group_list) ? (MenuGroup *)group_list->find(group) : 0;
    if (!grp) {
	// group does not already exist, create a new one
	grp = new MenuGroup(getRootNode(), group);
	if (grp) group_list->insert(group, grp);
    }

    // remind that we belong to the given group
    m_groups.append(group);

    // register this node as a child of the group
    if (grp) grp->registerChild(this);
}

//*****************************************************************************
void MenuNode::leaveGroup(const QString &group)
{
    QDict<MenuNode> *group_list = getGroupList();

    MenuGroup *grp = (group_list) ? (MenuGroup *)group_list->find(group) : 0;

    // remove the group from our list
    m_groups.remove(group);

    // remove ourself from the group
    if (grp) grp->removeChild(this);
}

//*****************************************************************************
bool MenuNode::specialCommand(const QString &command)
{
    if (command.startsWith("#group(")) {
	Parser parser(command);

	QString group = parser.firstParam();
	while (group.length()) {
	    joinGroup(group);
	    group = parser.nextParam();
	}
	return true;
    } else if (command.startsWith("#disable")) {
	// disable the node
	setEnabled(false);
	return true;
    } else if (command.startsWith("#enable")) {
	// disable the node
	setEnabled(true);
	return true;
    }
    return false;
}

/* end of MenuNode.cpp */
