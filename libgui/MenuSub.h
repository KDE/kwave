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

#ifndef MENUSUB_H
#define MENUSUB_H

#include "MenuNode.h"

class QPopupMenu;

/**
 * This is the class for submenu entries in a Menu. It is normally owned by a
 * root menu node, a toplevel menu or an other submenu.
 * @author Thomas Eschenbacher
 */
class MenuSub : public MenuNode
{
  Q_OBJECT

public: // Public methods
    MenuSub (const char *name);

    virtual int getChildIndex(const int id);
    virtual bool isBranch() {return true;};

    virtual MenuNode *insertBranch(char *name, const char *key,
                                   const char *uid, const int index=-1);

    virtual MenuNode *insertLeaf(const char *command, char *name,
                                 const char *key, const char *uid,
                                 const int index=-1);

    virtual QPopupMenu *getPopupMenu();

    virtual void removeChild(const int id);

    virtual bool specialCommand(const char *command);

private:
    QPopupMenu *menu;

};

#endif
