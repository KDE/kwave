#############################################################################
##    Kwave                - cmake/KwaveALSASupport.cmake
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

OPTION(WITH_ALSA "enable playback/recording via ALSA [default=on]" ON)

IF (WITH_ALSA)

    INCLUDE(FindALSA)
    FIND_PATH(HAVE_ASOUNDLIB_H alsa/asoundlib.h)

    IF (HAVE_ASOUNDLIB_H)
        SET(HAVE_ALSA_SUPPORT  ON CACHE BOOL "enable ALSA support")
    ELSE (HAVE_ASOUNDLIB_H)
        MESSAGE(FATAL_ERROR "Your system lacks ALSA support")
    ENDIF (HAVE_ASOUNDLIB_H)

ENDIF (WITH_ALSA)

#############################################################################
#############################################################################
