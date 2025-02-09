/***************************************************************************
               memcpy.h -  prototype for optimized memcpy
                     -------------------
    begin                : Tue Dec 07 2004
    copyright            : (C) 2004 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef MEMCPY_H
#define MEMCPY_H

#include "config.h"

#include <string.h> /* for memcpy from libc */

/** use standard memcpy() from libc */
#define MEMCPY memcpy

#endif /* MEMCPY_H */

//***************************************************************************
//***************************************************************************
