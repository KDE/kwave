/***************************************************************************
                          MenuItem.h  -  description
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

#ifndef _MENU_ITEM_H_
#define _MENU_ITEM_H_

#include "MenuNode.h"

/**
 * Base class for entries in a Menu. It is normally owned by a toplevel
 * menu or a submenu.
 * @author Thomas Eschenbacher
 */
class MenuItem : public MenuNode {
  Q_OBJECT

public: // Public methods

    MenuItem(MenuNode *parent, char *name, char *command=0,
             int key=0, char *uid=0);
    virtual int getIndex();
    virtual bool specialCommand(const char *command);
    virtual void setEnabled(bool enable);

};

#endif // _MENU_ITEM_H_
