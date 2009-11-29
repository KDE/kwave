#############################################################################
##    Kwave                - cmake/KwavePulseAudioSupport.cmake
##                           -------------------
##    begin                : Mon Sep 28 2009
##    copyright            : (C) 2009 by Thomas Eschenbacher
##    email                : Thomas.Eschenbacher@gmx.de
#############################################################################
#
#############################################################################
##                                                                          #
##    This program is free software; you can redistribute it and/or modify  #
##    it under the terms of the GNU General Public License as published by  #
##    the Free Software Foundation; either version 2 of the License, or     #
##    (at your option) any later version.                                   #
##                                                                          #
#############################################################################

OPTION(WITH_PULSEAUDIO "enable playback/recording via PulseAudio [default=on]" ON)

IF (WITH_PULSEAUDIO)

INCLUDE(FindPkgConfig)
INCLUDE(UsePkgConfig)

PKG_CHECK_MODULES(PULSEAUDIO REQUIRED libpulse>=0.9.15)
IF (NOT PULSEAUDIO_FOUND)
    MESSAGE(FATAL_ERROR "PulseAudio library not found")
ENDIF(NOT PULSEAUDIO_FOUND)

PKG_CHECK_MODULES(PULSEAUDIO libpulse)

# MESSAGE(STATUS "Found PulseAudio version ${PULSEAUDIO_VERSION}")
MESSAGE(STATUS "Found PulseAudio library in ${PULSEAUDIO_LIBDIR}")
MESSAGE(STATUS "Found PulseAudio headers in ${PULSEAUDIO_INCLUDEDIR}")

SET(HAVE_PULSEAUDIO_SUPPORT  ON CACHE BOOL "enable PulseAudio support")

ENDIF (WITH_PULSEAUDIO)

#############################################################################
#############################################################################
