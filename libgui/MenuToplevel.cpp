/***************************************************************************
                          MenuToplevel.cpp  -  description
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

#include <kapp.h>

#include "MenuNode.h"
#include "MenuSub.h"
#include "MenuToplevel.h"

MenuToplevel::MenuToplevel(MenuNode *parent, char *name,
                           char *command, int key, char *uid)
    :MenuSub(parent, name, command, key, uid)
{
}

/* end of libgui/MenuToplevel.cpp */