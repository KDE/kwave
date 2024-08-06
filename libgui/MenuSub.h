/***************************************************************************
              MenuSub.h  -  submenu
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

#ifndef MENU_SUB_H
#define MENU_SUB_H

#include "config.h"

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QString>

#include "libgui/MenuNode.h"

namespace Kwave
{
    /**
     * This is the class for submenu entries in a Menu. It is normally
     * owned by a root menu node, a toplevel menu or another submenu.
     */
    class MenuSub: public Kwave::MenuNode
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         * @param parent pointer to the node's parent (might be 0)
         * @param menu the already generated QMenu
         * @param name the non-localized name of the submenu
         * @param command the command to be sent when the submenu is
         *                selected (optional, default=0)
         * @param shortcut keyboard shortcut, (optional, default=0)
         * @param uid unique id string (optional, default=0)
         */
        MenuSub(Kwave::MenuNode *parent,
                QMenu *menu,
                const QString &name,
                const QString &command,
                const QKeySequence &shortcut,
                const QString &uid);

        /** Destructor */
        virtual ~MenuSub() override;

        /**
         * Always returns true, as the nodes of this type are branches.
         */
        virtual bool isBranch() const override { return true; }

        /**
         * Inserts a new branch node under the submenu. The new node
         * normally is (derived from) MenuSub.
         * @param name non-localized name of the node
         * @param command the command template used for creating commands of
         *                submenus (leafes) that don't have an own command
         *                but contain data for their parent's command.
         *                Should contain a %s that will be replaced by some
         *                data from a child entry. (this is used for
         *                menus with data selection lists like "recent files)
         *                If not used, pass 0.
         * @param shortcut keyboard shortcut, 0 if unused
         * @param uid unique id string (might be 0)
         * @return pointer to the new branch node
         */
        virtual Kwave::MenuSub *insertBranch(const QString &name,
                                             const QString &command,
                                             const QKeySequence &shortcut,
                                             const QString &uid)
                                             override;

        /**
         * Inserts a new leaf node under the submenu. The new node
         * normally is (derived from) MenuItem.
         * @param name non-localized name of the node
         * @param command the command to be sent when the node is
         *                selected (might be 0)
         * @param shortcut keyboard shortcut, 0 if unused
         * @param uid unique id string (might be 0)
         * @return pointer to the new leaf node
         */
        virtual Kwave::MenuNode *insertLeaf(const QString &name,
                                            const QString &command,
                                            const QKeySequence &shortcut,
                                            const QString &uid) override;

        /** Returns the corresponding menu action */
        virtual QAction *action() override {
            return (m_menu) ? m_menu->menuAction() : Q_NULLPTR;
        }

        /**
         * Removes a child node of the current node. If the child
         * was not found or is already removed this does nothing.
         * @param child pointer to the child node
         */
        virtual void removeChild(Kwave::MenuNode *child) override;

        /**
         * Handles/interpretes special menu commands.
         * @param command name of a menu node or command
         * @return true if the name was recognized as a command and handled
         */
        virtual bool specialCommand(const QString &command) override;

        /**
         * Shows/hides the current sub menu.
         * @param visible true to show the sub menu, false to hide
         */
        virtual void setVisible(bool visible) override;

        /**
         * Returns true if the node is enabled.
         */
        virtual bool isEnabled() override;

        /**
         * Enables/disables the current menu node.
         * @param enable true to enable the item, false to disable
         */
        virtual void setEnabled(bool enable) override;

        /** Returns the menu nodes' icon. */
        virtual const QIcon icon() override;

        /**
         * Sets a new icon of a menu node.
         * @param icon QPixmap with the icon
         */
        virtual void setIcon(const QIcon &icon) override;

    protected:
        friend class MenuRoot;

        /** return the pointer to our QMenu */
        virtual QMenu *menu() { return m_menu; }

    private:

        /** the QMenu that is controlled */
        QMenu *m_menu;

    };
}

#endif /* MENU_SUB_H */

//***************************************************************************
//***************************************************************************
