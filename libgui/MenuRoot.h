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

#ifndef MENUROOT_H
#define MENUROOT_H

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

    /** Inserts a new child node into the current structure. */
    virtual int insertNode(MenuNode *node);

    virtual MenuNode *insertBranch(char *name, const char *key,
                                   const char *uid);

private: // Private attributes

  /** reference to a KMenuBar */
  KMenuBar &menu_bar;

};

#endif
