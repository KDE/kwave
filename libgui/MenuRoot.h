/***************************************************************************
                          MenuRoot.h  -  description
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

#ifndef _MENU_ROOT_H_
#define _MENU_ROOT_H_ 1

#include "MenuNode.h"

class KMenuBar;

/**
 * This is the class for the root of a Menu (e.g. a MenuBar) that contains
 * all toplevel menues of a menu hierarchy.
 * @author Thomas Eschenbacher
 */
class MenuRoot : public MenuNode
{
  Q_OBJECT

public: // Public methods

    MenuRoot(KMenuBar &bar);

    virtual int getChildIndex(int id);

    virtual MenuNode *insertBranch(char *name, int key,
                                   char *uid, int index=-1);

    virtual MenuNode *insertLeaf(char *name, char *command,
                                 int key, char *uid,
                                 int index=-1);

    virtual void removeChild(int id);

    virtual bool setItemEnabled(int id, bool enable);

private: // Private attributes

  /** reference to a KMenuBar */
  KMenuBar &menu_bar;

};

#endif // _MENU_ROOT_H_
