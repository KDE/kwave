/***************************************************************************
			  MenuNode.cpp  -  generic menu node type
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

#include <QPixmap>

#include <kapplication.h>
#include <kiconloader.h>

#include "libkwave/Parser.h"
#include "MenuNode.h"
#include "MenuGroup.h"
#include "MenuRoot.h"
#include "MenuSub.h"

//*****************************************************************************
MenuNode::MenuNode(MenuNode *parent,
                   const QString &name,
                   const QString &command,
                   const QKeySequence &shortcut,
                   const QString &uid)
    :QObject(), m_children(), m_groups(), m_uid(uid), m_shortcut(shortcut),
     m_name(name), m_command(command), m_parentNode(parent)
{
}

//*****************************************************************************
MenuNode::~MenuNode()
{
    // leave all groups
    QStringList::iterator group = m_groups.begin();
    while (group != m_groups.end()) {
	leaveGroup(*group);
	group = m_groups.begin();
    }

    // remove all clients
    clear();

    // deregister from our parent
    if (m_parentNode) m_parentNode->removeChild(this);
}

//*****************************************************************************
void MenuNode::emitCommand(const QString &command)
{
    Q_ASSERT(command.length());
    if (!command.length()) return ;

    if (!parentNode()) {
	// no parent -> we are the root node -> we have to emit
	emit sigCommand(command);
    } else {
	// tell the root node to emit
	MenuNode *root = getRootNode();
	Q_ASSERT(root);
	if (root) root->emitCommand(command);
    }
}

//*****************************************************************************
void MenuNode::actionSelected()
{
    if (m_command.length()) emitCommand(m_command);
}

//*****************************************************************************
void MenuNode::clear()
{
    // remove all children
    while (!m_children.isEmpty()) {
	MenuNode *child = m_children.takeLast();
	delete child;
    }
}

//*****************************************************************************
MenuNode *MenuNode::parentNode()
{
    return m_parentNode;
}

//*****************************************************************************
MenuNode *MenuNode::getRootNode()
{
    return (m_parentNode) ? m_parentNode->getRootNode() : this;
}

//*****************************************************************************
const QIcon MenuNode::icon()
{
    static QIcon dummy;
    Q_ASSERT(dummy.isNull());
    return dummy;
}

//*****************************************************************************
void MenuNode::setIcon(const QIcon &icon)
{
    qWarning("MenuNode(%s)::setIcon(%p)",
	name().toLocal8Bit().data(), reinterpret_cast<const void *>(&icon));
}

//*****************************************************************************
bool MenuNode::isEnabled()
{
    // evaluate our own (individual) enable and our parent's enable state
    if ((m_parentNode != 0) && !m_parentNode->isEnabled())
	return false;

    // find  out if all our groups are enabled
    MenuNode *root = getRootNode();
    if (root) {
	foreach (QString group_name, m_groups) {
	    MenuNode *group = root->findUID(group_name);
	    if (group && group->inherits("MenuGroup")) {
		if (!(static_cast<MenuGroup *>(group))->isEnabled()) {
		    qDebug("MenuNode(%s).isEnabled(): group %s is disabled",
			   name().toLocal8Bit().data(),
			   group_name.toLocal8Bit().data());
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
    Q_UNUSED(enable);
}

//*****************************************************************************
void MenuNode::setChecked(bool check)
{
    Q_UNUSED(check);
}

//*****************************************************************************
void MenuNode::setText(const QString &text)
{
    Q_UNUSED(text);
}

//*****************************************************************************
void MenuNode::registerChild(MenuNode *node)
{
    if (node) m_children.append(node);
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

    foreach (MenuNode *child, m_children) {
	MenuNode *node = (child) ? child->findUID(uid) : 0;
	if (node) return node;    // found in child
    }

    return 0;    // nothing found :-(
}

//*****************************************************************************
MenuNode *MenuNode::findChild(const QString &name)
{
    Q_ASSERT(name.length());

    foreach (MenuNode *child, m_children) {
	if (child && (name == child->name()))
	    return child;
    }
    return 0;
}

//*****************************************************************************
void MenuNode::removeChild(MenuNode *child)
{
    if (child && !m_children.isEmpty())
	m_children.removeAll(child);
}

//*****************************************************************************
MenuSub *MenuNode::insertBranch(const QString &name,
                                const QString &command,
                                const QKeySequence &shortcut,
                                const QString &uid)
{
    Q_UNUSED(name);
    Q_UNUSED(command);
    Q_UNUSED(shortcut);
    Q_UNUSED(uid);
    return 0;
}

//*****************************************************************************
MenuNode *MenuNode::insertLeaf(const QString &name,
                               const QString &command,
                               const QKeySequence &shortcut,
                               const QString &uid)
{
    Q_UNUSED(name);
    Q_UNUSED(command);
    Q_UNUSED(shortcut);
    Q_UNUSED(uid);
    return 0;
}

//*****************************************************************************
void MenuNode::insertNode(const QString &name,
                          const QString &position,
                          const QString &command,
                          const QKeySequence &shortcut,
                          const QString &uid)
{
    int pos = 0;

    if (!position.length()) {
	qWarning("MenuNode::parseCommand: no position!");
	return;
    }

    // make working copies of name and position
    QString n(name);
    QString p(position);

    // at start of the parsing process ?
    if (!n.length()) {
	// split off the first token, separated by a slash
	pos = p.indexOf('/');
	if (pos < 0) pos = p.length();
    }
    n = position.left(pos);
    p.remove(0, pos+1);
    if ((n.length()) && (specialCommand(n))) {
	// no new branch, only a special command
	return;
    }

    if ((!p.length()) || (p[0] == '#')) {
	// end of the tree
	MenuNode *sub = findChild(n);
	if (sub) {
	    // a leaf with this name already exists
	    // -> maybe we want to set new properties
	    if (shortcut) sub->setShortcut(shortcut);

	    if (uid.length()) sub->setUID(uid);

	    if (p[0] == '#') sub->specialCommand(p);
	    return;
	} else {
	    // insert a new leaf
	    MenuNode *leaf = insertLeaf(n, command, shortcut, uid);
	    if (!leaf) return;

	    if (p[0] == '#') leaf->specialCommand(p);
	    return;
	}
    } else {
	// somewhere in the tree
	MenuNode *sub = findChild(n);
	if (!sub) {
	    sub = insertBranch(n, command, shortcut, uid);
	} else if ( !sub->isBranch() && (p[0] != '#')) {
	    // remove the "leaf" and insert a branch with
	    // the same properties
	    sub = leafToBranch(sub);
	} else if ( (p[0] == '#') || (p[0] == 0) ) {
	    // branch already exists and we are at the end of parsing
	    // -> maybe we want to set new properties
	    if (shortcut) sub->setShortcut(shortcut);
	    if (uid.length()) sub->setUID(uid);
	}

	if (sub) {
	    sub->insertNode(0, p, command, shortcut, uid);
	} else {
	    qDebug("MenuNode::insertNode: branch failed!");
	}
    }
}

//*****************************************************************************
MenuNode *MenuNode::leafToBranch(MenuNode *node)
{
    Q_ASSERT(node);
    Q_ASSERT(node != this);

    if (!node || (node==this)) return 0;

    // get the old properties
    bool old_enable           = node->isEnabled();
    QKeySequence old_shortcut = node->shortcut();
    QString old_uid           = node->uid();
    QIcon old_icon            = node->icon();
    QString name              = node->name();
    QString command           = node->command();
    QStringList old_groups    = node->m_groups;

    // remove the old child node
    removeChild(node);

    // insert the new branch
    MenuSub *sub = insertBranch(name, command, old_shortcut, old_uid);
    if (sub) {
	// join it to the same groups
	foreach (QString group, old_groups)
	    sub->joinGroup(group);

	// set the old icon
	if (!old_icon.isNull()) sub->setIcon(old_icon);

	// set the "enable"
	sub->setEnabled(old_enable);
    }

    // free the old node later.
    // IMPORTANT: we must not call "delete node" now, because we get called
    //            through leafToBranch(this) !
    MenuRoot::deleteLater(node);

    return sub;
}

//*****************************************************************************
QHash<QString, MenuGroup *> &MenuNode::getGroupList()
{
    static QHash<QString, MenuGroup *> _empty_list;
    Q_ASSERT(m_parentNode);
    return (m_parentNode) ? m_parentNode->getGroupList() : _empty_list;
}

//*****************************************************************************
void MenuNode::joinGroup(const QString &group)
{
    if (m_groups.contains(group))
	return ;    // already joined

    QHash<QString, MenuGroup *> &group_list = getGroupList();
    MenuGroup *grp = 0;
    if (group_list.contains(group)) {
	grp = group_list[group];
    } else {
	// group does not already exist, create a new one
	grp = new MenuGroup(getRootNode(), group);
	if (grp) group_list.insert(group, grp);
    }

    // remember that we now belong to the given group
    m_groups.append(group);

    // register this node as a child of the group
    if (grp) grp->registerChild(this);
}

//*****************************************************************************
void MenuNode::leaveGroup(const QString &group)
{
    QHash<QString, MenuGroup *> &group_list = getGroupList();
    MenuGroup *grp = (group_list.contains(group)) ?
	group_list.value(group) : 0;

    // remove the group from our list
    m_groups.removeAll(group);

    // remove ourself from the group
    if (grp) grp->removeChild(this);
}

//*****************************************************************************
bool MenuNode::specialCommand(const QString &command)
{

    if (command.startsWith("#icon(")) {
	// --- give the item an icon ---
	Parser parser(command);
	const QString &filename = parser.firstParam();
	if (filename.length()) {
	    // try to load from standard dirs
	    KIconLoader loader;
	    QIcon icon = loader.loadIcon(filename,
		KIconLoader::Small, 0, KIconLoader::DefaultState,
		QStringList(), 0, true);

	    if (!icon.isNull()) {
		setIcon(icon);
	    } else {
		qWarning("MenuNode '%s': icon '%s' not found !",
		    name().toLocal8Bit().data(),
		    filename.toLocal8Bit().data());
	    }
	}
	return true;
    }

    if (command.startsWith("#listmenu")) {
	// insert an empty submenu for the list items
	MenuNode *parent = parentNode();
	if (parent) parent->leafToBranch(this);

	return true;
    }

    if (command.startsWith("#group(")) {
	Parser parser(command);

	QString group = parser.firstParam();
	while (group.length()) {
	    joinGroup(group);
	    group = parser.nextParam();
	}
	return true;
    }

    if (command.startsWith("#disable")) {
	// disable the node
	setEnabled(false);
	return true;
    }

    if (command.startsWith("#enable")) {
	// disable the node
	setEnabled(true);
	return true;
    }

    return false;
}

//***************************************************************************
#include "MenuNode.moc"
//***************************************************************************
//***************************************************************************
