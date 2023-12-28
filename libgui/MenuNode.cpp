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

#include <new>

#include <QLatin1Char>
#include <QPixmap>

#include "libkwave/Parser.h"
#include "libkwave/String.h"

#include "libgui/MenuGroup.h"
#include "libgui/MenuList.h"
#include "libgui/MenuNode.h"
#include "libgui/MenuRoot.h"
#include "libgui/MenuSub.h"

//*****************************************************************************
Kwave::MenuNode::MenuNode(Kwave::MenuNode *parent,
                          const QString &name,
                          const QString &command,
                          const QKeySequence &shortcut,
                          const QString &uid)
    :QObject(), m_children(), m_groups(), m_uid(uid), m_shortcut(shortcut),
     m_name(name), m_command(command), m_parentNode(parent)
{
}

//*****************************************************************************
Kwave::MenuNode::~MenuNode()
{
    // leave all groups
    QStringList::iterator group = m_groups.begin();
    while (group != m_groups.end()) {
	leaveGroup(*group);
	group = m_groups.begin();
    }

    // remove all childs
    clear();

    // de-register from our parent
    if (m_parentNode) m_parentNode->removeChild(this);
}

//*****************************************************************************
const QString Kwave::MenuNode::path() const
{
    return (m_parentNode) ?
	(m_parentNode->path() + _("/") + m_name) : QString();
}

//*****************************************************************************
void Kwave::MenuNode::emitCommand(const QString &command)
{
    Q_ASSERT(command.length());
    if (!command.length()) return ;

    if (!parentNode()) {
	// no parent -> we are the root node -> we have to emit
	emit sigCommand(command);
    } else {
	// tell the root node to emit
	Kwave::MenuNode *root = rootNode();
	Q_ASSERT(root);
	if (root) root->emitCommand(command);
    }
}

//*****************************************************************************
void Kwave::MenuNode::actionSelected()
{
    if (m_command.length()) emitCommand(m_command);
}

//*****************************************************************************
void Kwave::MenuNode::clear()
{
    // remove all children
    while (!m_children.isEmpty()) {
	Kwave::MenuNode *child = m_children.takeLast();
	delete child;
    }
}

//*****************************************************************************
Kwave::MenuNode *Kwave::MenuNode::parentNode() const
{
    return m_parentNode;
}

//*****************************************************************************
Kwave::MenuNode *Kwave::MenuNode::rootNode()
{
    return (m_parentNode) ? m_parentNode->rootNode() : this;
}

//*****************************************************************************
const QIcon Kwave::MenuNode::icon()
{
    static QIcon dummy;
    Q_ASSERT(dummy.isNull());
    return dummy;
}

//*****************************************************************************
void Kwave::MenuNode::setIcon(const QIcon &icon)
{
    qWarning("MenuNode(%s)::setIcon(%p)",
	DBG(name()), reinterpret_cast<const void *>(&icon));
}

//*****************************************************************************
bool Kwave::MenuNode::isEnabled()
{
    // evaluate our own (individual) enable and our parent's enable state
    if (m_parentNode && !m_parentNode->isEnabled())
	return false;

    // find  out if all our groups are enabled
    QHash<QString, Kwave::MenuGroup *> &groups = groupList();
    Kwave::MenuNode *root = rootNode();
    if (root) {
	foreach (const QString &group_name, m_groups) {
	    if (groups.contains(group_name)) {
		Kwave::MenuGroup *group = groups[group_name];
		if (group && !group->isEnabled()) {
		    qDebug("MenuNode(%s).isEnabled(): group %s is disabled",
			   DBG(name()), DBG(group_name));
		    return false;
		}
	    }
	}
    }

    // if we get here, everything is enabled
    return true;
}

//*****************************************************************************
void Kwave::MenuNode::setVisible(bool visible)
{
    Q_UNUSED(visible)
}

//*****************************************************************************
void Kwave::MenuNode::setEnabled(bool enable)
{
    Q_UNUSED(enable)
}

//*****************************************************************************
void Kwave::MenuNode::setChecked(bool check)
{
    Q_UNUSED(check)
}

//*****************************************************************************
void Kwave::MenuNode::setText(const QString &text)
{
    Q_UNUSED(text)
}

//*****************************************************************************
void Kwave::MenuNode::insertChild(Kwave::MenuNode *node,
                                  Kwave::MenuNode *before)
{
    if (!node) return;
    if (before && m_children.contains(before))
	m_children.insert(m_children.indexOf(before), node);
    else
	m_children.append(node);
}

//*****************************************************************************
void Kwave::MenuNode::setUID(const QString &uid)
{
    m_uid = uid;
}

//*****************************************************************************
Kwave::MenuNode *Kwave::MenuNode::findUID(const QString &uid)
{
    if (m_uid == uid) return this;    // found ourself

    foreach (Kwave::MenuNode *child, m_children) {
        Kwave::MenuNode *node = (child) ? child->findUID(uid) : Q_NULLPTR;
	if (node) return node;    // found in child
    }

    return Q_NULLPTR;    // nothing found :-(
}

//*****************************************************************************
Kwave::MenuNode *Kwave::MenuNode::findChild(const QString &name)
{
    Q_ASSERT(name.length());

    foreach (Kwave::MenuNode *child, m_children) {
	if (child && (name == child->name()))
	    return child;
    }
    return Q_NULLPTR;
}

//*****************************************************************************
void Kwave::MenuNode::removeChild(Kwave::MenuNode *child)
{
    if (child && !m_children.isEmpty())
	m_children.removeAll(child);
}

//*****************************************************************************
Kwave::MenuSub *Kwave::MenuNode::insertBranch(const QString &name,
                                              const QString &command,
                                              const QKeySequence &shortcut,
                                              const QString &uid)
{
    Q_UNUSED(name)
    Q_UNUSED(command)
    Q_UNUSED(shortcut)
    Q_UNUSED(uid)
    return Q_NULLPTR;
}

//*****************************************************************************
Kwave::MenuNode *Kwave::MenuNode::insertLeaf(const QString &name,
                                             const QString &command,
                                             const QKeySequence &shortcut,
                                             const QString &uid)
{
    Q_UNUSED(name)
    Q_UNUSED(command)
    Q_UNUSED(shortcut)
    Q_UNUSED(uid)
    return Q_NULLPTR;
}

//*****************************************************************************
void Kwave::MenuNode::insertNode(const QString &name,
                                 const QString &position,
                                 const QString &command,
                                 const QKeySequence &shortcut,
                                 const QString &uid)
{
    int pos = 0;

    if (!position.length()) {
	qWarning("MenuNode::insertNode: no position!");
	return;
    }

    // at start of the parsing process ?
    if (!name.length()) {
	// split off the first token, separated by a slash
	pos = position.indexOf(QLatin1Char('/'));
	if (pos < 0) pos = position.length();
    }

    QString n = position.left(pos);
    QString p = position;
    p.remove(0, pos + 1);

    if ((n.length()) && (specialCommand(n))) {
	// no new branch, only a special command
	return;
    }

    if ((!p.length()) || (p[0] == QLatin1Char('#'))) {
	// end of the tree
	Kwave::MenuNode *sub = findChild(n);
	if (sub) {
	    // a leaf with this name already exists
	    // -> maybe we want to set new properties
	    if (!shortcut.isEmpty()) sub->setShortcut(shortcut);

	    if (uid.length()) sub->setUID(uid);

	} else {
	    // insert a new leaf
	    sub = insertLeaf(n, command, shortcut, uid);
	    if (!sub) return;
	}

	if (p.length() && (p[0] == QLatin1Char('#')))
	    sub->specialCommand(p);

    } else {
	// somewhere in the tree
	Kwave::MenuNode *sub = findChild(n);
	if (!sub) {
	    sub = insertBranch(n, command, shortcut, uid);
	} else if ( !sub->isBranch() && (p[0] != QLatin1Char('#'))) {
	    // remove the "leaf" and insert a branch with
	    // the same properties
	    sub = leafToBranch(sub);
	} else if (p[0] == QLatin1Char('#')) {
	    // branch already exists and we are at the end of parsing
	    // -> maybe we want to set new properties
	    if (!shortcut.isEmpty()) sub->setShortcut(shortcut);
	    if (uid.length()) sub->setUID(uid);
	}

	if (sub) {
	    sub->insertNode(QString(), p, command, shortcut, uid);
	} else {
	    qDebug("MenuNode::insertNode: branch failed!");
	}
    }
}

//*****************************************************************************
Kwave::MenuNode *Kwave::MenuNode::leafToBranch(Kwave::MenuNode *node)
{
    Q_ASSERT(node);
    Q_ASSERT(node != this);

    if (!node || (node == this)) return Q_NULLPTR;

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
    Kwave::MenuSub *sub = insertBranch(name, command, old_shortcut, old_uid);
    if (sub) {
	// join it to the same groups
	foreach (const QString &group, old_groups)
	    sub->joinGroup(group, Kwave::MenuGroup::NORMAL);

	// set the old icon
	if (!old_icon.isNull()) sub->setIcon(old_icon);

	// set the "enable"
	sub->setEnabled(old_enable);
    }

    // free the old node later.
    // IMPORTANT: we must not call "delete node" now, because we get called
    //            through leafToBranch(this) !
    Kwave::MenuRoot::deleteLater(node);

    return sub;
}

//*****************************************************************************
QHash<QString, Kwave::MenuGroup *> &Kwave::MenuNode::groupList()
{
    static QHash<QString, Kwave::MenuGroup *> _empty_list;
    Q_ASSERT(m_parentNode);
    return (m_parentNode) ? m_parentNode->groupList() : _empty_list;
}

//*****************************************************************************
void Kwave::MenuNode::joinGroup(const QString &group,
                                Kwave::MenuGroup::Mode mode)
{
    if (m_groups.contains(group))
	return;    // already joined

    QHash<QString, Kwave::MenuGroup *> &group_list = groupList();
    Kwave::MenuGroup *grp = Q_NULLPTR;
    if (group_list.contains(group)) {
	grp = group_list[group];
    } else {
	// group does not already exist, create a new one
        grp = new(std::nothrow) Kwave::MenuGroup(rootNode(), group, mode);
	if (grp) group_list.insert(group, grp);
    }

    // remember that we now belong to the given group
    m_groups.append(group);

    // register this node as a child of the group
    if (grp) grp->join(this);
}

//*****************************************************************************
void Kwave::MenuNode::leaveGroup(const QString &group)
{
    QHash<QString, Kwave::MenuGroup *> &group_list = groupList();
    Kwave::MenuGroup *grp = (group_list.contains(group)) ?
	group_list.value(group) : Q_NULLPTR;

    // remove the group from our list
    m_groups.removeAll(group);

    // remove ourself from the group
    if (grp) {
	grp->leave(this);

	// clean up the group if nobody uses it any more
	if (grp->isEmpty()) delete grp;
    }
}

//*****************************************************************************
bool Kwave::MenuNode::specialCommand(const QString &command)
{
    Kwave::Parser parser(command);

    if (parser.command() == _("#icon")) {
	// --- give the item an icon ---
	const QString &icon_name = parser.firstParam();
	if ( icon_name.length()) {
	    // try to load from standard dirs
	    QIcon icon = QIcon::fromTheme( icon_name );
	    if (!icon.isNull()) {
		setIcon(icon);
	    } else {
		qWarning("MenuNode '%s': icon '%s' not found !",
		    DBG(name()), DBG( icon_name ));
	    }
	}
	return true;
    }

    if (parser.command() == _("#listmenu")) {
	// make sure that the current node is a sub menu
	Kwave::MenuNode *parent = parentNode();
	Kwave::MenuNode *sub = (parent && !isBranch()) ?
	    parent->leafToBranch(this) : this;
	if (!sub) return false;

	// append a placeholder for inserting the list
	// (if it does not already exist)
	const QString uid = parser.firstParam();
	const QString cmd = parser.nextParam();
	if (!sub->findUID(uid)) {
            Kwave::MenuList *placeholder =
                new(std::nothrow) Kwave::MenuList(sub, cmd, uid);
	    Q_ASSERT(placeholder);
	    if (!placeholder) return false;

            sub->insertChild(placeholder, Q_NULLPTR);
	}
	return true;
    }

    if (parser.command() == _("#group")) {
	QString group = parser.firstParam();
	while (group.length()) {
	    joinGroup(group, Kwave::MenuGroup::NORMAL);
	    group = parser.nextParam();
	}
	return true;
    }

    if (command == _("#disabled")) {
	// disable the node
	setEnabled(false);
	return true;
    }

    if (command == _("#enabled")) {
	// enable the node
	setEnabled(true);
	return true;
    }

    return false;
}

//***************************************************************************
//***************************************************************************
