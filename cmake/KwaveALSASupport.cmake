#############################################################################
##    Kwave                - cmake/KwaveALSASupport.cmake
##                           -------------------
##    begin                : Sat Jun 02 2007
##    copyright            : (C) 2007 by Thomas Eschenbacher
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

OPTION(WITH_ALSA "enable playback/recording via ALSA [default=on]" ON)

IF (WITH_ALSA)

    CHECK_INCLUDE_FILES(alsa/asoundlib.h HAVE_ASOUNDLIB_H)

    IF (HAVE_ASOUNDLIB_H)
        MESSAGE(STATUS "Enabled ALSA for playbay and recording")
        SET(HAVE_ALSA_SUPPORT  ON CACHE BOOL "enable ALSA support")
    ELSE (HAVE_ASOUNDLIB_H)
        MESSAGE(FATAL_ERROR "Your system lacks ALSA support")
    ENDIF (HAVE_ASOUNDLIB_H)

ENDIF (WITH_ALSA)

#############################################################################
#############################################################################
