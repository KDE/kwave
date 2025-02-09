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

/* support playback/recording via ALSA */
#cmakedefine HAVE_ALSA_SUPPORT

/* enable the debug plugin in the menu */
#cmakedefine HAVE_DEBUG_PLUGIN

/* support playback/recording via PulseAudio */
#cmakedefine HAVE_PULSEAUDIO_SUPPORT

/* support playback via Qt */
#cmakedefine HAVE_QT_AUDIO_SUPPORT

/* support libaudiofile */
#cmakedefine HAVE_LIBAUDIOFILE

/* support libsamplerate */
#cmakedefine HAVE_LIBSAMPLERATE

/* support playback/recording via OSS */
#cmakedefine HAVE_OSS_SUPPORT

/* Define to 1 if you have the <signal.h> header file. */
#cmakedefine HAVE_SIGNAL_H

/* we can include <sys/times.h> */
#cmakedefine HAVE_SYS_TIMES_H

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
#cmakedefine PROJECT_NAME "@PROJECT_NAME@"

/* version of the application */
#cmakedefine KWAVE_VERSION "@KWAVE_VERSION@"

/* suffix of executable files */
#cmakedefine EXECUTABLE_SUFFIX @CMAKE_EXECUTABLE_SUFFIX@

/* libaudiofile knows FLAC compression */
#cmakedefine HAVE_AF_COMPRESSION_FLAC

/***************************************************************************/
/***************************************************************************/
