/***************************************************************************
               cputest.h -  defines with CPU capabilites
                     -------------------
    begin                : Mon Dec 06 2004
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

#ifndef _CPUTEST_H_
#define _CPUTEST_H_

/* x86 accelerations */
#define MM_ACCEL_X86_MMX        0x80000000
#define MM_ACCEL_X86_3DNOW      0x40000000
#define MM_ACCEL_X86_MMXEXT     0x20000000
#define MM_ACCEL_X86_SSE        0x10000000
#define MM_ACCEL_X86_SSE2       0x08000000
#define MM_ACCEL_X86_SSE3       0x04000000
#define MM_ACCEL_X86_SSSE3      0x02000000
#define MM_ACCEL_X86_SSE4       0x01000000
#define MM_ACCEL_X86_SSE42      0x00800000
#define MM_ACCEL_X86_AVX        0x00400000

/* x86 compat defines */
#define MM_MMX                  MM_ACCEL_X86_MMX
#define MM_3DNOW                MM_ACCEL_X86_3DNOW
#define MM_MMXEXT               MM_ACCEL_X86_MMXEXT
#define MM_SSE                  MM_ACCEL_X86_SSE
#define MM_SSE2                 MM_ACCEL_X86_SSE2

#ifdef __cplusplus
extern "C" {
#endif

uint32_t xine_mm_accel (void);

#ifdef __cplusplus
}
#endif

#endif /* _CPUTEST_H_ */

/***************************************************************************/
/***************************************************************************/
