/***************************************************************************
                          MenuGroup.h  -  controls a group of menu nodes
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
#ifndef MENU_GROUP_H
#define MENU_GROUP_H

#include "config.h"

#include <QList>

class QActionGroup;

namespace Kwave
{

    class MenuNode;

    /**
     * A MenuGroup controls a group of menu nodes (items, submenus).
     */
    class MenuGroup
    {
    public:

        /** mode for group membership */
        typedef enum
        {
            NORMAL,   /**< normal group, no dependencies */
            EXCLUSIVE /**< exclusive group, one of many (radio buttons) */
        } Mode;

        /**
         * Constructor.
         * @param parent pointer to the group's parent (might be 0)
         * @param name the unique name of the group
         * @param mode the mode of the group, normal or exclusive
         */
        MenuGroup(Kwave::MenuNode *parent,
                  const QString &name,
                  Kwave::MenuGroup::Mode mode);

        /**
         * Destructor. cleans up.
         * @see #clear()
         */
        virtual ~MenuGroup();

        /**
         * add a menu node to the group
         * @param node a MenuNode to join, must not be NULL
         */
        virtual void join(Kwave::MenuNode *node);

        /**
         * remove a menu node from the group
         * @param node a MenuNode to remove, must not be NULL
         */
        virtual void leave(Kwave::MenuNode *node);

        /**
         * Enables/disables all members of the group.
         * @param enable true to enable the item, false to disable
         */
        virtual void setEnabled(bool enable);

        /**
         * returns the "enabled" state of the group
         */
        virtual bool isEnabled() const { return m_enabled; }

        /**
         * returns true if the group contains no members
         */
        virtual bool isEmpty() const { return m_members.isEmpty(); }

        /**
         * Resets all checkmarks of the group members except the one member
         * that will get the new selected one. If no new member id is given
         * no member will get selected. This method is useful for making
         * exclusive selections of menu items.
         * @param uid the unique id string of the member to be selected or 0
         */
        virtual void selectItem(const QString &uid);

        /**
         * De-registers all child nodes from us and removes them from
         * our internal list of child nodes.
         */
        virtual void clear();

    private:

        /** the parent menu node */
        Kwave::MenuNode *m_parent = nullptr;

        /** name of the group */
        QString m_name;

        /** list of group members */
        QList<Kwave::MenuNode *> m_members;

        /** a QActionGroup, in case of "exclusive" mode */
        QActionGroup *m_action_group = nullptr;

        /** the group's enabled/disabled flag */
        bool m_enabled = true;

    };
}

#endif // MENU_GROUP_H

//***************************************************************************
//***************************************************************************
