/***************************************************************************
                          MenuSub.h  -  description
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

#ifndef _MENU_SUB_H_
#define _MENU_SUB_H_ 1

#include "MenuItem.h"

class QPixmap;
class QPopupMenu;

/**
 * This is the class for submenu entries in a Menu. It is normally owned by a
 * root menu node, a toplevel menu or an other submenu.
 * @author Thomas Eschenbacher
 */
class MenuSub : public MenuItem
{
  Q_OBJECT

public: // Public methods
    MenuSub(MenuNode *parent, char *name, char *command=0,
            int key=0, char *uid=0);

    virtual int getChildIndex(int id);
    virtual bool isBranch() {return true;};

    virtual MenuNode *insertBranch(char *name, char *command, int key,
                                   char *uid, int index=-1);

    virtual MenuNode *insertLeaf(char *name, char *command,
                                 int key, char *uid,
                                 int index=-1);

    virtual QPopupMenu *getPopupMenu();

    virtual void removeChild(MenuNode *child);

    virtual bool specialCommand(const char *command);

    virtual void setItemIcon(int id, const QPixmap &icon);

    virtual void setItemChecked(int id, bool check);

    /**
     * Informs the node that the enabled state of a child node
     * might have changed.
     * @param id menu id of the child node
     * @param enable true if the item has been enabled, false if disabled
     */
    virtual void actionChildEnableChanged(int id, bool enable);

public slots:

    void slotSelected(int);

private:

    QPopupMenu *menu;
};

#endif // _MENU_SUB_H_
