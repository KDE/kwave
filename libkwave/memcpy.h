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
#ifndef _MEMCPY_H_
#define _MEMCPY_H_

#include "config.h"

#ifdef HAVE_OPTIMIZED_MEMCPY

#include <stdlib.h> /* for size_t */

/** use optimized memcpy() from xine */
#define MEMCPY xine_fast_memcpy

/* forward declaration to libkwave/memcpy.c */
extern "C" void *(* xine_fast_memcpy)(void *to, const void *from, size_t len);

#else /* HAVE_OPTIMIZED_MEMCPY */

#include <string.h> /* for memcpy from ligc */

/** use standard memcpy() from libc */
#define MEMCPY memcpy

#endif /* HAVE_OPTIMIZED_MEMCPY */

#endif /* _MEMCPY_H_ */
