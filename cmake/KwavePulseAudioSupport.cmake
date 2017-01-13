#############################################################################
##    Kwave                - cmake/KwavePulseAudioSupport.cmake
##                           -------------------
##    begin                : Mon Sep 28 2009
##    copyright            : (C) 2009 by Thomas Eschenbacher
##    email                : Thomas.Eschenbacher@gmx.de
#############################################################################
#
#############################################################################
#                                                                           #
# Redistribution and use in source and binary forms, with or without        #
# modification, are permitted provided that the following conditions        #
# are met:                                                                  #
#                                                                           #
# 1. Redistributions of source code must retain the above copyright         #
#    notice, this list of conditions and the following disclaimer.          #
# 2. Redistributions in binary form must reproduce the above copyright      #
#    notice, this list of conditions and the following disclaimer in the    #
#    documentation and/or other materials provided with the distribution.   #
#                                                                           #
# For details see the accompanying cmake/COPYING-CMAKE-SCRIPTS file.        #
#                                                                           #
#############################################################################

OPTION(WITH_PULSEAUDIO "enable playback/recording via PulseAudio [default=on]" ON)

IF (WITH_PULSEAUDIO)

    INCLUDE(FindPkgConfig)
    INCLUDE(UsePkgConfig)

    PKG_CHECK_MODULES(PULSEAUDIO REQUIRED libpulse>=0.9.16)

    MESSAGE(STATUS "Found PulseAudio library in ${PULSEAUDIO_LIBDIR}")
    MESSAGE(STATUS "Found PulseAudio headers in ${PULSEAUDIO_INCLUDEDIR}")

    SET(HAVE_PULSEAUDIO_SUPPORT  ON CACHE BOOL "enable PulseAudio support")

ENDIF (WITH_PULSEAUDIO)

#############################################################################
#############################################################################
