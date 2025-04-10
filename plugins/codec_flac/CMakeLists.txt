#############################################################################
##    Kwave                - plugins/codec_flac/CMakeLists.txt
##                           -------------------
##    begin                : Mon May 28 2007
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

OPTION(WITH_FLAC "enable support for FLAC files [default=on]" ON)
IF (WITH_FLAC)

    PKG_CHECK_MODULES(FLAC REQUIRED flac>=1.2.0)
    MESSAGE(STATUS "Found FLAC version ${FLAC_VERSION}")

    PKG_CHECK_MODULES(FLAC++ REQUIRED flac++>=1.2.0)
    MESSAGE(STATUS "Found FLAC++ version ${FLAC++_VERSION}")

    SET(HAVE_FLAC  ON CACHE BOOL "enable FLAC codec")
    SET(plugin_codec_flac_LIB_SRCS
        FlacCodecPlugin.cpp
        FlacDecoder.cpp
        FlacEncoder.cpp

        FlacCodecPlugin.h
        FlacDecoder.h
        FlacEncoder.h
    )

    SET(plugin_codec_flac_LIBS
        ${FLAC_LINK_LIBRARIES}
        ${FLAC++_LINK_LIBRARIES}
    )

    KWAVE_PLUGIN(codec_flac)

ENDIF(WITH_FLAC)

#############################################################################
#############################################################################
