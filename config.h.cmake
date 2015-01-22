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

/* use target optimized memcpy */
#cmakedefine WITH_OPTIMIZED_MEMCPY

/* use target optimized memcpy using AVX assembler code */
#cmakedefine HAVE_AVX

/* support playback/recording via ALSA */
#cmakedefine HAVE_ALSA_SUPPORT

/* enable the debug plugin in the menu */
#cmakedefine HAVE_DEBUG_PLUGIN

/* support playback/recording via PulseAudio */
#cmakedefine HAVE_PULSEAUDIO_SUPPORT

/* support playback via Phonon */
#cmakedefine HAVE_PHONON_SUPPORT

/* support libsamplerate */
#cmakedefine HAVE_LIBSAMPLERATE

/* resource limits for the memory manager */
#cmakedefine HAVE_GETRLIMIT

/* sysinfo is needed for the memory manager */
#cmakedefine HAVE_SYSINFO

/* sysinfo structure has the "mem_unit" field? */
#cmakedefine HAVE_SYSINFO_MEMUNIT

/* used for page size in context of swap files */
#cmakedefine HAVE_GETPAGESIZE

/* used for determining the page size in context of swap file handling */
#cmakedefine HAVE_SYSCONF

/* support playback/recording via OSS */
#cmakedefine HAVE_OSS_SUPPORT

/* Define to 1 if you have the <signal.h> header file. */
#cmakedefine HAVE_SIGNAL_H

/* we can include <sys/times.h> */
#cmakedefine HAVE_SYS_TIMES_H

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H

/* used for unlinking swap files */
#cmakedefine HAVE_UNLINK

/* support FLAC */
#cmakedefine HAVE_FLAC

/* support MP3 */
#cmakedefine HAVE_MP3

/* does libogg have the function ogg_stream_flush_fill ? (>= v1.3.0) */
#cmakedefine HAVE_OGG_STREAM_FLUSH_FILL

/* support Vorbis in Ogg */
#cmakedefine HAVE_OGG_VORBIS

/* support Opus in Ogg */
#cmakedefine HAVE_OGG_OPUS

/* Name of package */
#cmakedefine PACKAGE "@PACKAGE@"

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION "@PACKAGE_VERSION@"

/* suffix of executable files */
#cmakedefine EXECUTABLE_SUFFIX @CMAKE_EXECUTABLE_SUFFIX@

/* enable memory management debug code */
#cmakedefine DEBUG_MEMORY

/***************************************************************************/
/***************************************************************************/
