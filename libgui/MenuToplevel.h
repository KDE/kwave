/***************************************************************************
                          MenuToplevel.h  -  description
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

#ifndef MENUTOPLEVEL_H
#define MENUTOPLEVEL_H

#include "MenuSub.h"

/**
 * This is the class for toplevel submenu entries in a Menu. It is normally 
 * owned by a root menu node
 * @author Thomas Eschenbacher
 */
class MenuToplevel : public MenuSub
{
  Q_OBJECT

public: // Public methods
    MenuToplevel (const char *name);
    // virtual bool setEnabled();

};

#endif
