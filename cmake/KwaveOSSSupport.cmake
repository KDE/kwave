#############################################################################
##    Kwave                - cmake/KwaveOSSSupport.cmake
##                           -------------------
##    begin                : Sat Jun 02 2007
##    copyright            : (C) 2007 by Thomas Eschenbacher
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
