/*************************************************************************
             byteswap.h  -  platform independend byteswap macros
                             -------------------
    begin                : Fri Jul 31 2004
    copyright            : (C) 2004 by Thomas Eschenbacher
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
#ifndef _BYTESWAP_H_
#define _BYTESWAP_H_

#include "config.h"

/***************************************************************************/
#ifdef HAVE_BYTESWAP_H
/* use byteswap macros from the host system, hopefully optimized ones ;-) */
#include <byteswap.h>
#else
/* define our own version, simple, stupid, straight-forward... */

#define bswap_16(x)   ((((x) & 0xFF00) >> 8) | (((x) & 0xFF00) << 8))

#define bswap_32(x)   ((((x) & 0xFF000000) >> 24) | \
                       (((x) & 0x00FF0000) >> 8)  | \
		       (((x) & 0x0000FF00) << 8)  | \
		       (((x) & 0x000000FF) << 24) )

#endif

#endif /* _BYTESWAP_H_ */

/***************************************************************************/
/***************************************************************************/
