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

#include <QtCore/QLatin1Char>
#include <QtGui/QKeySequence>
#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtGui/QWidget>

#include <klocale.h>

#include "libkwave/Parser.h"
#include "libkwave/String.h"

#include "libgui/MenuNode.h"
#include "libgui/MenuRoot.h"
#include "libgui/MenuGroup.h"
#include "libgui/MenuManager.h"

//***************************************************************************
/** static map with standard keys */
QMap<QString, QKeySequence> Kwave::MenuManager::m_standard_keys;

/** helper macro for inserting an entry into the m_standard_keys map */
#define _INS(n,v) m_standard_keys.insert(_(n), v)

//***************************************************************************
Kwave::MenuManager::MenuManager(QWidget *parent, KMenuBar &bar)
    :QObject(parent)
{
    m_menu_root = new Kwave::MenuRoot(bar);
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
    // awk '{ print "\t_INS(\""$0"\", QKeySequence::"$0");"}'

    if (m_standard_keys.isEmpty()) {
	_INS("UnknownKey",            QKeySequence::UnknownKey);
	_INS("HelpContents",          QKeySequence::HelpContents);
	_INS("WhatsThis",             QKeySequence::WhatsThis);
	_INS("Open",                  QKeySequence::Open);
	_INS("Close",                 QKeySequence::Close);
	_INS("Save",                  QKeySequence::Save);
	_INS("New",                   QKeySequence::New);
	_INS("Delete",                QKeySequence::Delete);
	_INS("Cut",                   QKeySequence::Cut);
	_INS("Copy",                  QKeySequence::Copy);
	_INS("Paste",                 QKeySequence::Paste);
	_INS("Undo",                  QKeySequence::Undo);
	_INS("Redo",                  QKeySequence::Redo);
	_INS("Back",                  QKeySequence::Back);
	_INS("Forward",               QKeySequence::Forward);
	_INS("Refresh",               QKeySequence::Refresh);
	_INS("ZoomIn",                QKeySequence::ZoomIn);
	_INS("ZoomOut",               QKeySequence::ZoomOut);
	_INS("Print",                 QKeySequence::Print);
	_INS("AddTab",                QKeySequence::AddTab);
	_INS("NextChild",             QKeySequence::NextChild);
	_INS("PreviousChild",         QKeySequence::PreviousChild);
	_INS("Find",                  QKeySequence::Find);
	_INS("FindNext",              QKeySequence::FindNext);
	_INS("FindPrevious",          QKeySequence::FindPrevious);
	_INS("Replace",               QKeySequence::Replace);
	_INS("SelectAll",             QKeySequence::SelectAll);
	_INS("Bold",                  QKeySequence::Bold);
	_INS("Italic",                QKeySequence::Italic);
	_INS("Underline",             QKeySequence::Underline);
	_INS("MoveToNextChar",        QKeySequence::MoveToNextChar);
	_INS("MoveToPreviousChar",    QKeySequence::MoveToPreviousChar);
	_INS("MoveToNextWord",        QKeySequence::MoveToNextWord);
	_INS("MoveToPreviousWord",    QKeySequence::MoveToPreviousWord);
	_INS("MoveToNextLine",        QKeySequence::MoveToNextLine);
	_INS("MoveToPreviousLine",    QKeySequence::MoveToPreviousLine);
	_INS("MoveToNextPage",        QKeySequence::MoveToNextPage);
	_INS("MoveToPreviousPage",    QKeySequence::MoveToPreviousPage);
	_INS("MoveToStartOfLine",     QKeySequence::MoveToStartOfLine);
	_INS("MoveToEndOfLine",       QKeySequence::MoveToEndOfLine);
	_INS("MoveToStartOfBlock",    QKeySequence::MoveToStartOfBlock);
	_INS("MoveToEndOfBlock",      QKeySequence::MoveToEndOfBlock);
	_INS("MoveToStartOfDocument", QKeySequence::MoveToStartOfDocument);
	_INS("MoveToEndOfDocument",   QKeySequence::MoveToEndOfDocument);
	_INS("SelectNextChar",        QKeySequence::SelectNextChar);
	_INS("SelectPreviousChar",    QKeySequence::SelectPreviousChar);
	_INS("SelectNextWord",        QKeySequence::SelectNextWord);
	_INS("SelectPreviousWord",    QKeySequence::SelectPreviousWord);
	_INS("SelectNextLine",        QKeySequence::SelectNextLine);
	_INS("SelectPreviousLine",    QKeySequence::SelectPreviousLine);
	_INS("SelectNextPage",        QKeySequence::SelectNextPage);
	_INS("SelectPreviousPage",    QKeySequence::SelectPreviousPage);
	_INS("SelectStartOfLine",     QKeySequence::SelectStartOfLine);
	_INS("SelectEndOfLine",       QKeySequence::SelectEndOfLine);
	_INS("SelectStartOfBlock",    QKeySequence::SelectStartOfBlock);
	_INS("SelectEndOfBlock",      QKeySequence::SelectEndOfBlock);
	_INS("SelectStartOfDocument", QKeySequence::SelectStartOfDocument);
	_INS("SelectEndOfDocument",   QKeySequence::SelectEndOfDocument);
	_INS("DeleteStartOfWord",     QKeySequence::DeleteStartOfWord);
	_INS("DeleteEndOfWord",       QKeySequence::DeleteEndOfWord);
	_INS("DeleteEndOfLine",       QKeySequence::DeleteEndOfLine);
    }
}

//***************************************************************************
void Kwave::MenuManager::executeCommand(const QString &command)
{

    Q_ASSERT(command.length());
    if (!m_menu_root) return; // makes no sense if no menu root

    Kwave::Parser parser(command);

    QKeySequence shortcut;  // keyboard shortcut (optional)
    QString id ;            // string id (optional)
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
	QRegExp rx(_("::(\\w+)"), Qt::CaseInsensitive);
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
		qWarning("MenuManager::executeCommand: pos=%d, stdname='%s' "
		         "-> UNKNOWN ???", pos, DBG(stdname));
		break;
	    }
	}

	// default case: direct specification of a key sequence
	shortcut = QKeySequence::fromString(i18n(param.toAscii()));
    }

    // --- 4rth parameter: parse the string id of the node (optional) ---
    param = parser.nextParam();
    if (param.length()) id = param;

#ifdef DEBUG
//     qDebug("MenuManager: insertNode('', '%s', '%s', %s, '%s')",
// 	   DBG(pos), DBG(com), DBG(shortcut.toString()), DBG(id));
    if (!shortcut.isEmpty()) {
	static QMap<QString, QString> used_shortcuts;

	QString sc = shortcut.toString();
	QString m  = pos.left(pos.indexOf(_("/#")));

	if (used_shortcuts.contains(sc) && (used_shortcuts[sc] != m)) {
	    qWarning("MenuManager: insertNode('%s')", DBG(m));
	    qWarning("    shortcut %s already in use for '%s'",
		     DBG(sc), DBG(used_shortcuts[sc]));
	} else {
	    used_shortcuts[sc] = m;
	}
    }
#endif /* DEBUG */

    // --- insert the new node into the menu structure ---
    m_menu_root->insertNode(QString(), pos, com, shortcut, id);
}

//***************************************************************************
void Kwave::MenuManager::clearNumberedMenu(const QString &uid)
{
    Kwave::MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    if (node) node->clear();
}

//***************************************************************************
void Kwave::MenuManager::addNumberedMenuEntry(const QString &uid,
	const QString &entry)
{
    Q_ASSERT(entry.length());
    if (!entry.length()) return;

    Q_ASSERT(m_menu_root);
    Kwave::MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    if (node) {
	QString cmd     = node->command();
	QString command = cmd.contains(_("%1")) ? cmd.arg(entry) : cmd;

	node->insertLeaf(entry, command, 0, QString());
    } else
	qWarning("MenuManager: could not find numbered Menu '%s'", DBG(uid));

}

//***************************************************************************
void Kwave::MenuManager::selectItem(const QString &group, const QString &uid)
{
    if (!group.length()) {
	qWarning("MenuManager::selectItem('','%s'): no group!?", DBG(uid));
	return ;
    }

    if (group[0] != QLatin1Char('@')) {
	qWarning("MenuManager::selectItem('%s','%s'): "
		"invalid group name, does not start with '@'!",
		DBG(group), DBG(uid));
	return ;
    }

    Kwave::MenuNode *node = (m_menu_root) ? m_menu_root->findUID(group) : 0;
    if (!node) {
	qWarning("MenuManager::selectItem(): group '%s' not found!",
	         DBG(group));
	return ;
    }

    if (!qobject_cast<Kwave::MenuGroup *>(node)) {
	qWarning("MenuManager::selectItem(): '%s' is not a group!", DBG(group));
	return ;
    }

    (reinterpret_cast<Kwave::MenuGroup *>(node))->selectItem(uid);
}

//***************************************************************************
void Kwave::MenuManager::setItemChecked(const QString &uid, bool check)
{
    Kwave::MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    if (node) node->setChecked(check);
}

//***************************************************************************
void Kwave::MenuManager::setItemText(const QString &uid, const QString &text)
{
    Kwave::MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : 0;
    if (node) node->setText(text);
}

//***************************************************************************
void Kwave::MenuManager::setItemEnabled(const QString &uid, bool enable)
{
    if (!m_menu_root) return;

    Kwave::MenuNode *node = m_menu_root->findUID(uid);
    if (node) node->setEnabled(enable);
    else qWarning("MenuManager::setItemEnabled('%s', '%d'): uid not found!",
		  DBG(uid), enable);
}

//***************************************************************************
Kwave::MenuManager::~MenuManager()
{
    delete m_menu_root;
    m_menu_root = 0;
}

//***************************************************************************
#include "MenuManager.moc"
//***************************************************************************
//***************************************************************************
