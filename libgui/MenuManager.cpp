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

#include <errno.h>
#include <stdlib.h>

#include <new>

#include <QKeySequence>
#include <QLatin1Char>
#include <QRegExp>
#include <QString>
#include <QWidget>

#include <KLocalizedString>

#include "libkwave/Parser.h"
#include "libkwave/String.h"

#include "libgui/MenuGroup.h"
#include "libgui/MenuManager.h"
#include "libgui/MenuNode.h"
#include "libgui/MenuRoot.h"

//***************************************************************************
/** static map with standard keys */
QMap<QString, QKeySequence> Kwave::MenuManager::m_standard_keys;

/** helper macro for inserting an entry into the m_standard_keys map */
#define INS(n,v) m_standard_keys.insert(_(n), v)

//***************************************************************************
Kwave::MenuManager::MenuManager(QWidget *parent, QMenuBar &bar)
    :QObject(parent)
{
    m_menu_root = new(std::nothrow) Kwave::MenuRoot(bar);
    Q_ASSERT(m_menu_root);
    if (m_menu_root) {
        connect(
            m_menu_root, SIGNAL(sigCommand(QString)),
            this, SIGNAL(sigMenuCommand(QString)),
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
    // awk '{ print "\tINS(\""$0"\", QKeySequence::"$0");"}'

    if (m_standard_keys.isEmpty()) {
        INS("UnknownKey",            QKeySequence::UnknownKey);
        INS("HelpContents",          QKeySequence::HelpContents);
        INS("WhatsThis",             QKeySequence::WhatsThis);
        INS("Open",                  QKeySequence::Open);
        INS("Close",                 QKeySequence::Close);
        INS("Save",                  QKeySequence::Save);
        INS("New",                   QKeySequence::New);
        INS("Delete",                QKeySequence::Delete);
        INS("Cut",                   QKeySequence::Cut);
        INS("Copy",                  QKeySequence::Copy);
        INS("Paste",                 QKeySequence::Paste);
        INS("Undo",                  QKeySequence::Undo);
        INS("Redo",                  QKeySequence::Redo);
        INS("Back",                  QKeySequence::Back);
        INS("Forward",               QKeySequence::Forward);
        INS("Refresh",               QKeySequence::Refresh);
        INS("ZoomIn",                QKeySequence::ZoomIn);
        INS("ZoomOut",               QKeySequence::ZoomOut);
        INS("Print",                 QKeySequence::Print);
        INS("AddTab",                QKeySequence::AddTab);
        INS("NextChild",             QKeySequence::NextChild);
        INS("PreviousChild",         QKeySequence::PreviousChild);
        INS("Find",                  QKeySequence::Find);
        INS("FindNext",              QKeySequence::FindNext);
        INS("FindPrevious",          QKeySequence::FindPrevious);
        INS("Replace",               QKeySequence::Replace);
        INS("SelectAll",             QKeySequence::SelectAll);
        INS("Bold",                  QKeySequence::Bold);
        INS("Italic",                QKeySequence::Italic);
        INS("Underline",             QKeySequence::Underline);
        INS("MoveToNextChar",        QKeySequence::MoveToNextChar);
        INS("MoveToPreviousChar",    QKeySequence::MoveToPreviousChar);
        INS("MoveToNextWord",        QKeySequence::MoveToNextWord);
        INS("MoveToPreviousWord",    QKeySequence::MoveToPreviousWord);
        INS("MoveToNextLine",        QKeySequence::MoveToNextLine);
        INS("MoveToPreviousLine",    QKeySequence::MoveToPreviousLine);
        INS("MoveToNextPage",        QKeySequence::MoveToNextPage);
        INS("MoveToPreviousPage",    QKeySequence::MoveToPreviousPage);
        INS("MoveToStartOfLine",     QKeySequence::MoveToStartOfLine);
        INS("MoveToEndOfLine",       QKeySequence::MoveToEndOfLine);
        INS("MoveToStartOfBlock",    QKeySequence::MoveToStartOfBlock);
        INS("MoveToEndOfBlock",      QKeySequence::MoveToEndOfBlock);
        INS("MoveToStartOfDocument", QKeySequence::MoveToStartOfDocument);
        INS("MoveToEndOfDocument",   QKeySequence::MoveToEndOfDocument);
        INS("SelectNextChar",        QKeySequence::SelectNextChar);
        INS("SelectPreviousChar",    QKeySequence::SelectPreviousChar);
        INS("SelectNextWord",        QKeySequence::SelectNextWord);
        INS("SelectPreviousWord",    QKeySequence::SelectPreviousWord);
        INS("SelectNextLine",        QKeySequence::SelectNextLine);
        INS("SelectPreviousLine",    QKeySequence::SelectPreviousLine);
        INS("SelectNextPage",        QKeySequence::SelectNextPage);
        INS("SelectPreviousPage",    QKeySequence::SelectPreviousPage);
        INS("SelectStartOfLine",     QKeySequence::SelectStartOfLine);
        INS("SelectEndOfLine",       QKeySequence::SelectEndOfLine);
        INS("SelectStartOfBlock",    QKeySequence::SelectStartOfBlock);
        INS("SelectEndOfBlock",      QKeySequence::SelectEndOfBlock);
        INS("SelectStartOfDocument", QKeySequence::SelectStartOfDocument);
        INS("SelectEndOfDocument",   QKeySequence::SelectEndOfDocument);
        INS("DeleteStartOfWord",     QKeySequence::DeleteStartOfWord);
        INS("DeleteEndOfWord",       QKeySequence::DeleteEndOfWord);
        INS("DeleteEndOfLine",       QKeySequence::DeleteEndOfLine);
    }
}

//***************************************************************************
int Kwave::MenuManager::executeCommand(const QString &command)
{

    Q_ASSERT(command.length());
    if (!m_menu_root) return -EINVAL; // makes no sense if no menu root

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
        return -EINVAL;
    }

    // --- 3rd parameter: bitmask for the key shortcut (optional) ---
    param = parser.nextParam();
    if (param.length()) {
        // replace "::<StandardKeyName>" with the key sequence as string
        QRegExp rx(_("::(\\w+)"), Qt::CaseInsensitive);
        int p = 0;
        while ((p = rx.indexIn(param, 0)) >= 0) {
            QString stdname = rx.cap(1);
            if (m_standard_keys.contains(stdname)) {
                // translate into a key sequence
                QKeySequence sequence = m_standard_keys[stdname];
                QString expanded = sequence.toString();
                param = param.replace(p, stdname.length() + 2, expanded);
            } else {
                // unknown standard key sequence name?
                qWarning("MenuManager::executeCommand: pos=%d, stdname='%s' "
                         "-> UNKNOWN ???", p, DBG(stdname));
                break;
            }
        }

        // default case: direct specification of a key sequence
        shortcut = QKeySequence::fromString(i18n(param.toLatin1()));
    }

    // --- 4rth parameter: parse the string id of the node (optional) ---
    param = parser.nextParam();
    if (param.length()) id = param;

#ifdef DEBUG
//     qDebug("MenuManager: insertNode('', '%s', '%s', %s, '%s')",
//         DBG(pos), DBG(com), DBG(shortcut.toString()), DBG(id));
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

    return 0;
}

//***************************************************************************
void Kwave::MenuManager::clearNumberedMenu(const QString &uid)
{
    Kwave::MenuNode *node = (m_menu_root) ?
        m_menu_root->findUID(uid) : Q_NULLPTR;
    if (node) node->clear();
}

//***************************************************************************
void Kwave::MenuManager::addNumberedMenuEntry(const QString &uid,
                                              const QString &entry,
                                              const QString &param)
{
    Q_ASSERT(entry.length());
    if (!entry.length()) return;

    Q_ASSERT(m_menu_root);
    Kwave::MenuNode *node = (m_menu_root) ? m_menu_root->findUID(uid) : Q_NULLPTR;
    Kwave::MenuNode *parent = (node) ? node->parentNode() : Q_NULLPTR;
    if (parent) {
        QString cmd = node->command();
        if (cmd.contains(_("%1"))) {
            QString p = (param.length()) ? param : entry;
            cmd = cmd.arg(Kwave::Parser::escape(p));
        }
        node->insertLeaf(entry, cmd, 0, uid);
    } else
        qWarning("MenuManager: could not find numbered Menu '%s'", DBG(uid));

}

//***************************************************************************
void Kwave::MenuManager::selectItem(const QString &group, const QString &uid)
{
    if (!m_menu_root)
        return;

    if (!group.length()) {
        qWarning("MenuManager::selectItem('','%s'): no group!?", DBG(uid));
        return;
    }

    if (group[0] != QLatin1Char('@')) {
        qWarning("MenuManager::selectItem('%s','%s'): "
                "invalid group name, does not start with '@'!",
                DBG(group), DBG(uid));
        return;
    }

    QHash<QString, Kwave::MenuGroup *> &groups = m_menu_root->groupList();
    if (!groups.contains(group)) {
        qWarning("MenuManager::selectItem(): group '%s' not found!",
                 DBG(group));
        return;
    }

    Kwave::MenuGroup *group_node = groups[group];
    Q_ASSERT(group_node);
    if (group_node) group_node->selectItem(uid);
}

//***************************************************************************
void Kwave::MenuManager::setItemChecked(const QString &uid, bool check)
{
    Kwave::MenuNode *node = (m_menu_root) ?
        m_menu_root->findUID(uid) : Q_NULLPTR;
    if (node) node->setChecked(check);
}

//***************************************************************************
void Kwave::MenuManager::setItemText(const QString &uid, const QString &text)
{
    Kwave::MenuNode *node = (m_menu_root) ?
        m_menu_root->findUID(uid) : Q_NULLPTR;
    if (node) node->setText(text);
}

//***************************************************************************
void Kwave::MenuManager::setItemVisible(const QString &uid, bool show)
{
    if (!m_menu_root) return;

    Kwave::MenuNode *node = m_menu_root->findUID(uid);
    if (node) {
        /* show/hide a single menu node */
        node->setVisible(show);
    } else {
        qWarning("MenuManager::setItemVisible('%s', '%d'): uid not found!",
                 DBG(uid), show);
    }
}

//***************************************************************************
void Kwave::MenuManager::setItemEnabled(const QString &uid, bool enable)
{
    if (!m_menu_root) return;

    bool found = false;
    Kwave::MenuNode *node = m_menu_root->findUID(uid);
    if (node) {
        /* enable/disable a single menu node */
        node->setEnabled(enable);
        found = true;
    } else {
        /* enable/disable a group */
        QHash<QString, Kwave::MenuGroup *> &groups = m_menu_root->groupList();
        if (groups.contains(uid)) {
            Kwave::MenuGroup *group = groups[uid];
            if (group) {
                group->setEnabled(enable);
                found = true;
            }
        }
    }

    if (!found)
        qWarning("MenuManager::setItemEnabled('%s', '%d'): uid not found!",
                 DBG(uid), enable);
}

//***************************************************************************
Kwave::MenuManager::~MenuManager()
{
    delete m_menu_root;
    m_menu_root = Q_NULLPTR;
}

//***************************************************************************
//***************************************************************************
