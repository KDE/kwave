/***************************************************************************
    config.h.cmake  -  template config.h (cmake build system)
                             -------------------
    begin                : Sun Jun 10 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
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

/* use ppc target optimizations (from xine) */
#cmakedefine ARCH_PPC

/* use ix86 target optimizations (from xine) */
#cmakedefine ARCH_X86

/* use X86_64 target optimizations (from xine) */
#cmakedefine ARCH_X86_64

/* byte-order of the host machine */
#cmakedefine ENDIANESS_BIG

/* byte-order of the host machine */
#cmakedefine ENDIANESS_LITTLE

/* FLAC API of v1.1.1 and before */
#cmakedefine FLAC_API_VERSION_1_1_1_OR_OLDER

/* FLAC API of v1.1.2 */
#cmakedefine FLAC_API_VERSION_1_1_2

/* FLAC API of v1.1.3 */
#cmakedefine FLAC_API_VERSION_1_1_3

/* FLAC API of v1.1.4 */
#cmakedefine FLAC_API_VERSION_1_1_4

/* support playback/recording via ALSA */
#cmakedefine HAVE_ALSA_SUPPORT

/* support playback via aRts */
#cmakedefine HAVE_ARTS_SUPPORT

/* Define to 1 if you have the <byteswap.h> header file. */
#cmakedefine HAVE_BYTESWAP_H

/* resource limits for the memory manager */
#cmakedefine HAVE_GETRLIMIT

/* meminfo is needed for the memory manager */
#cmakedefine HAVE_MEMINFO

/* used for creating swap files */
#cmakedefine HAVE_MKSTEMP

/* support playback/recording via OSS */
#cmakedefine HAVE_OSS_SUPPORT

/* we can include <sys/times.h> */
#cmakedefine HAVE_SYS_TIMES_H

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H

/* used for unlinking swap files */
#cmakedefine HAVE_UNLINK

/* Name of package */
#cmakedefine PACKAGE "@PACKAGE@"

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION "@PACKAGE_VERSION@"

/* The size of `long', as computed by sizeof. */
#cmakedefine SIZEOF_LONG @SIZEOF_LONG@

/* The size of `size_t', as computed by sizeof. */
#cmakedefine SIZEOF_SIZE_T @SIZEOF_SIZE_T@

/* use own copy of libaudiofile */
#cmakedefine USE_BUILTIN_LIBAUDIOFILE

/***************************************************************************/
/***************************************************************************/
