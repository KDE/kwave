/***************************************************************************
               cputest.h -  defines with CPU capabilites
                     -------------------
    begin                : Mon Dec 06 2004
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef _CPUTEST_H_
#define _CPUTEST_H_

#define MM_MMX    0x0001 /**< standard MMX */
#define MM_3DNOW  0x0004 /**< AMD 3DNOW */
#define MM_MMXEXT 0x0002 /**< SSE integer functions or AMD MMX ext */
#define MM_SSE    0x0008 /**< SSE functions */
#define MM_SSE2   0x0010 /**< PIV SSE2 functions */

#endif /* _CPUTEST_H_ */
