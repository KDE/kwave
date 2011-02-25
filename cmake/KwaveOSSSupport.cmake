#############################################################################
##    Kwave                - cmake/KwaveOSSSupport.cmake
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

OPTION(WITH_OSS "enable playback/recording via OSS [default=on]" ON)

IF (WITH_OSS)

    SET(_oss_inc sys/ioctl.h fcntl.h sys/soundcard.h)
    CHECK_INCLUDE_FILES("${_oss_inc}" HAVE_SYS_SOUNDCARD_H)

    IF (HAVE_SYS_SOUNDCARD_H)
        MESSAGE(STATUS "Enabled OSS for playback and recording")
        SET(HAVE_OSS_SUPPORT  ON CACHE BOOL "enable OSS support")
    ELSE (HAVE_SYS_SOUNDCARD_H)
        MESSAGE(FATAL_ERROR "Your system lacks OSS support")
    ENDIF (HAVE_SYS_SOUNDCARD_H)

ENDIF (WITH_OSS)

#############################################################################
#############################################################################
