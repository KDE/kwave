/***************************************************************************
          MenuManager.h  -  manager class for Kwave's menu structure
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

#ifndef _MENU_MANAGER_H_
#define _MENU_MANAGER_H_

#include "config.h"

#include <QtCore/QObject>
#include <QtCore/QMap>

#include <kdemacros.h>

class QString;
class QWidget;
class KMenuBar;

namespace Kwave
{

    class MenuRoot;

    /**
     * @class MenuManager
     * @brief Manager class for access to Kwave's menu subsystem.
     *
     * @note All commands must be emitted synchronously during X11 event
     *       processing instead of immediately through normal signal
     *       handling. This avoids trouble when a signal handler within
     *       the MenuNode class causes an action that deletes that menu
     *       node. <em>It took me one week to find that bug!</em>
     */
    class KDE_EXPORT MenuManager: public QObject
    {
	Q_OBJECT
    public:
	/**
	 * Constructor.
	 * @param parent the menu's parent widget
	 * @param bar reference to the menu bar
	 */
	MenuManager(QWidget *parent, KMenuBar &bar);

	/** Destructor. */
	virtual ~MenuManager();

	/**
	 * Executes menu commands.
	 * @param command string with the command
	 * @retval 0 if succeeded
	 * @retval -EINVAL if failed
	 */
	int executeCommand(const QString &command);

	/**
	 * Deletes all entries of a numbered menu
	 * @param uid unique id string of the numbered menu
	 */
	void clearNumberedMenu(const QString &uid);

	/**
	 * Add an entry to a numbered menu
	 * @param uid unique id string of the numbered menu
	 * @param entry name of the new entry (non-localized)
	 */
	void addNumberedMenuEntry(const QString &uid, const QString &entry);

	/**
	 * Selects an menu item within a group of menu items. All other
	 * items will be deselected and the new one will become the only
	 * selected one. (exclusive one-of-n-selection)
	 * @param group name of the menu group
	 * @param uid unique id string specifying the new selection
	 */
	void selectItem(const QString &group, const QString &uid);

	/**
	 * Sets the text of a menu entry to a new value.
	 * @param uid unique id string of the menu node
	 * @param text the new text of the item
	 */
	void setItemText(const QString &uid, const QString &text);

	/**
	 * Shows/hides a menu entry identified by an ID. Groups are
	 * not supported.
	 * @param uid unique id string of the menu node
	 * @param show true to show, false to hide
	 */
	void setItemVisible(const QString &uid, bool show);

    public slots:

	/**
	 * Checks/unchecks a menu node.
	 * @param uid unique id string of the menu node
	 * @param check true to set a checkmark, false to remove
	 */
	void setItemChecked(const QString &uid, bool check);

	/**
	 * Enables/disables a menu node.
	 * @param uid unique id string of the menu node
	 * @param enable true to enable, false to disable
	 */
	void setItemEnabled(const QString &uid, bool enable);

    signals:

	/**
	 * Will be emitted if the command of a menu node
	 * should be executed.
	 * @see MenuNode.sigCommand()
	 */
	void sigMenuCommand(const QString &command);

    private:

	/** root node of the menu structure */
	Kwave::MenuRoot *m_menu_root;

	/** map of standard key names / key sequences */
	static QMap<QString, QKeySequence> m_standard_keys;

    };
}

#endif // _MENU_MANAGER_H_

//***************************************************************************
//***************************************************************************
