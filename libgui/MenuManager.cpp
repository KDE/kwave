/***************************************************************************
        MenuManager.cpp  -  manager class for Kwave's menu structure
			     -------------------
    begin                : Sun Jun 4 2000
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
#include <stdlib.h>

#include <QKeySequence>
#include <QRegExp>
#include <QString>
#include <QWidget>

#include <klocale.h>

#include "libkwave/Parser.h"

#include "MenuNode.h"
#include "MenuRoot.h"
#include "MenuGroup.h"
#include "MenuManager.h"

//***************************************************************************
/** static map with standard keys */
QMap<QString, QKeySequence> MenuManager::m_standard_keys;

//***************************************************************************
MenuManager::MenuManager(QWidget *parent, KMenuBar &bar)
    :QObject(parent)
{
    m_menu_root = new MenuRoot(bar);
    Q_ASSERT(m_menu_root);
    if (m_menu_root) {
	connect(
	    m_menu_root, SIGNAL(sigCommand(const QString &)),
	    this, SIGNAL(sigMenuCommand(const QString &)),
	    Qt::QueuedConnection
	);
    }

    // fill the list of standard keys if it is empty.
    //
    // HINT: I have gerated the list below through copy&paste of the
    //       description of "enum QKeySequence::StandardKey" into
    //       list.txt and then:
    //
    // cat list.txt | sed s/^\ \ //g | cut -d \  -f 1 | cut -d \: -f 3 |
    // awk '{ print "\tm_standard_keys.insert(\""$0"\", QKeySequence::"$0");"}'

    if (m_standard_keys.isEmpty()) {
	m_standard_keys.insert("UnknownKey", QKeySequence::UnknownKey);
	m_standard_keys.insert("HelpContents", QKeySequence::HelpContents);
	m_standard_keys.insert("WhatsThis", QKeySequence::WhatsThis);
	m_standard_keys.insert("Open", QKeySequence::Open);
	m_standard_keys.insert("Close", QKeySequence::Close);
	m_standard_keys.insert("Save", QKeySequence::Save);
	m_standard_keys.insert("New", QKeySequence::New);
	m_standard_keys.insert("Delete", QKeySequence::Delete);
	m_standard_keys.insert("Cut", QKeySequence::Cut);
	m_standard_keys.insert("Copy", QKeySequence::Copy);
	m_standard_keys.insert("Paste", QKeySequence::Paste);
	m_standard_keys.insert("Undo", QKeySequence::Undo);
	m_standard_keys.insert("Redo", QKeySequence::Redo);
	m_standard_keys.insert("Back", QKeySequence::Back);
	m_standard_keys.insert("Forward", QKeySequence::Forward);
	m_standard_keys.insert("Refresh", QKeySequence::Refresh);
	m_standard_keys.insert("ZoomIn", QKeySequence::ZoomIn);
	m_standard_keys.insert("ZoomOut", QKeySequence::ZoomOut);
	m_standard_keys.insert("Print", QKeySequence::Print);
	m_standard_keys.insert("AddTab", QKeySequence::AddTab);
	m_standard_keys.insert("NextChild", QKeySequence::NextChild);
	m_standard_keys.insert("PreviousChild", QKeySequence::PreviousChild);
	m_standard_keys.insert("Find", QKeySequence::Find);
	m_standard_keys.insert("FindNext", QKeySequence::FindNext);
	m_standard_keys.insert("FindPrevious", QKeySequence::FindPrevious);
	m_standard_keys.insert("Replace", QKeySequence::Replace);
	m_standard_keys.insert("SelectAll", QKeySequence::SelectAll);
	m_standard_keys.insert("Bold", QKeySequence::Bold);
	m_standard_keys.insert("Italic", QKeySequence::Italic);
	m_standard_keys.insert("Underline", QKeySequence::Underline);
	m_standard_keys.insert("MoveToNextChar", QKeySequence::MoveToNextChar);
	m_standard_keys.insert("MoveToPreviousChar", QKeySequence::MoveToPreviousChar);
	m_standard_keys.insert("MoveToNextWord", QKeySequence::MoveToNextWord);
	m_standard_keys.insert("MoveToPreviousWord", QKeySequence::MoveToPreviousWord);
	m_standard_keys.insert("MoveToNextLine", QKeySequence::MoveToNextLine);
	m_standard_keys.insert("MoveToPreviousLine", QKeySequence::MoveToPreviousLine);
	m_standard_keys.insert("MoveToNextPage", QKeySequence::MoveToNextPage);
	m_standard_keys.insert("MoveToPreviousPage", QKeySequence::MoveToPreviousPage);
	m_standard_keys.insert("MoveToStartOfLine", QKeySequence::MoveToStartOfLine);
	m_standard_keys.insert("MoveToEndOfLine", QKeySequence::MoveToEndOfLine);
	m_standard_keys.insert("MoveToStartOfBlock", QKeySequence::MoveToStartOfBlock);
	m_standard_keys.insert("MoveToEndOfBlock", QKeySequence::MoveToEndOfBlock);
	m_standard_keys.insert("MoveToStartOfDocument", QKeySequence::MoveToStartOfDocument);
	m_standard_keys.insert("MoveToEndOfDocument", QKeySequence::MoveToEndOfDocument);
	m_standard_keys.insert("SelectNextChar", QKeySequence::SelectNextChar);
	m_standard_keys.insert("SelectPreviousChar", QKeySequence::SelectPreviousChar);
	m_standard_keys.insert("SelectNextWord", QKeySequence::SelectNextWord);
	m_standard_keys.insert("SelectPreviousWord", QKeySequence::SelectPreviousWord);
	m_standard_keys.insert("SelectNextLine", QKeySequence::SelectNextLine);
	m_standard_keys.insert("SelectPreviousLine", QKeySequence::SelectPreviousLine);
	m_standard_keys.insert("SelectNextPage", QKeySequence::SelectNextPage);
	m_standard_keys.insert("SelectPreviousPage", QKeySequence::SelectPreviousPage);
	m_standard_keys.insert("SelectStartOfLine", QKeySequence::SelectStartOfLine);
	m_standard_keys.insert("SelectEndOfLine", QKeySequence::SelectEndOfLine);
	m_standard_keys.insert("SelectStartOfBlock", QKeySequence::SelectStartOfBlock);
	m_standard_keys.insert("SelectEndOfBlock", QKeySequence::SelectEndOfBlock);
	m_standard_keys.insert("SelectStartOfDocument", QKeySequence::SelectStartOfDocument);
	m_standard_keys.insert("SelectEndOfDocument", QKeySequence::SelectEndOfDocument);
	m_standard_keys.insert("DeleteStartOfWord", QKeySequence::DeleteStartOfWord);
	m_standard_keys.insert("DeleteEndOfWord", QKeySequence::DeleteEndOfWord);
	m_standard_keys.insert("DeleteEndOfLine", QKeySequence::DeleteEndOfLine);
    }
}

//***************************************************************************
void MenuManager::executeCommand(const QString &command)
{

    Q_ASSERT(command.length());
    Q_ASSERT(m_menu_root);
    if (!m_menu_root) return; // makes no sense if no menu root

    Parser parser(command);

    QKeySequence shortcut;  // keyboard shortcut (optional)
    QString id = 0;         // string id (optional)
    QString param;

    // --- 1st parameter: command to be sent when selected ---
    QString com = parser.firstParam();

    // --- 2nd parameter: position in the menu structure ---
    QString pos = parser.nextParam();

    // bail out if no menu position is found
    if (!pos.length()) {
	qWarning("no position field !");
	return ;
    }

    // --- 3rd parameter: bitmask for the key shortcut (optional) ---
    param = parser.nextParam();
    if (param.length()) {
	// replace "::<StandardKeyName>" with the key sequence as string
	QRegExp rx("::(\\w+)", Qt::CaseInsensitive);
	int pos = 0;
	while ((pos = rx.indexIn(param, 0)) >= 0) {
	    QString stdname = rx.cap(1);
	    if (m_standard_keys.contains(stdname)) {
		// translate into a key sequence
		QKeySequence sequence = m_standard_keys[stdname];
		QString expanded = sequence.toString();
		param = param.replace(pos, stdname.length()+2, expanded);
	    } else {
		// unknown standard key sequence name?
		qWarning("MenuManager::executeCommand: pos=%d, stdname='%s' "\
		         "-> UNKNOWN ???", pos, stdname.toLocal8Bit().data());
		break;
	    }
	}

	// default case: direct specification of a key sequence
	shortcut = QKeySequence::fromString(i18n(param.toAscii()));
    }

    // --- 4rth parameter: parse the string id of the node (optional) ---
    param = parser.nextParam();
    if (param.length()) id = param;

    // --- insert the new node into the menu structure ---
    m_menu_root->insertNode(0, pos, com, shortcut, id);
}

//***************************************************************************
void MenuManager::clearNumberedMenu(const QString &uid)
{
    Q_ASSERT(m_menu_root);
    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    if (node) node->clear();
}

//***************************************************************************
void MenuManager::addNumberedMenuEntry(const QString &uid,
	const QString &entry)
{
    Q_ASSERT(entry.length());
    if (!entry.length()) return;

    Q_ASSERT(m_menu_root);
    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    if (node) {
	QString cmd = node->command();
	QString command = cmd.contains("%1") ? cmd.arg(entry) : cmd;

	node->insertLeaf(entry, command, 0, 0);
    } else
	qWarning("MenuManager: could not find numbered Menu '%s'",
	         uid.toLocal8Bit().data());

}

//***************************************************************************
void MenuManager::selectItem(const QString &group, const QString &uid)
{
    Q_ASSERT(m_menu_root);

    if (!group.length()) {
	qWarning("MenuManager::selectItem('','%s'): no group!?",
	         uid.toLocal8Bit().data());
	return ;
    }

    if (group[0] != '@') {
	qWarning("MenuManager::selectItem('%s','%s'): "\
		"invalid group name, does not start with '@'!",
		group.toLocal8Bit().data(), uid.toLocal8Bit().data());
	return ;
    }

    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(group) : 0;
    if (!node) {
	qWarning("MenuManager::selectItem(): group '%s' not found!",
	    group.toLocal8Bit().data());
	return ;
    }

    if (!node->inherits("MenuGroup")) {
	qWarning("MenuManager::selectItem(): '%s' is not a group!",
	    group.toLocal8Bit().data());
	return ;
    }

    (reinterpret_cast<MenuGroup *>(node))->selectItem(uid);
}

//***************************************************************************
void MenuManager::setItemChecked(const QString &uid, bool check)
{
    Q_ASSERT(m_menu_root);
    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    if (node) node->setChecked(check);
}

//***************************************************************************
void MenuManager::setItemText(const QString &uid, const QString &text)
{
    Q_ASSERT(m_menu_root);
    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    if (node) node->setText(text);
}

//***************************************************************************
void MenuManager::setItemEnabled(const QString &uid, bool enable)
{
    Q_ASSERT(m_menu_root);
    MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    if (node) node->setEnabled(enable);
    else qWarning("MenuManager::setItemEnabled('%s', '%d'): uid not found!",
		  uid.toLocal8Bit().data(), enable);
}

//***************************************************************************
MenuManager::~MenuManager()
{
    Q_ASSERT(m_menu_root);
    if (m_menu_root) delete m_menu_root;
}

//***************************************************************************
#include "MenuManager.moc"
//***************************************************************************
//***************************************************************************
