#############################################################################
##    Kwave                - plugins/codec_ogg/CMakeLists.txt
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

OPTION(WITH_OGG_VORBIS "enable support for Ogg/Vorbis files [default=on]" ON)
OPTION(WITH_OGG_OPUS   "enable support for Ogg/Opus files [default=on]" ON)

IF (WITH_OGG_OPUS OR WITH_OGG_VORBIS)

#############################################################################
### check for Ogg headers and library                                     ###

    INCLUDE(FindPkgConfig)
    INCLUDE(UsePkgConfig)

    PKG_CHECK_MODULES(OGG REQUIRED ogg>=1.0.0)
    MESSAGE(STATUS "  Found Ogg library in ${OGG_LIBDIR}")
    MESSAGE(STATUS "  Found Ogg headers in ${OGG_INCLUDEDIR}")
    SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${OGG_LIBRARIES})

#############################################################################
### check for Opus headers and library                                    ###

    IF (WITH_OGG_OPUS)
        PKG_CHECK_MODULES(OPUS REQUIRED opus>=1.0.0)
        MESSAGE(STATUS "  Found opus codec library in ${OPUS_LIBDIR}")
        MESSAGE(STATUS "  Found opus codec headers in ${OPUS_INCLUDEDIR}")
        SET(CMAKE_REQUIRED_INCLUDES
            ${CMAKE_REQUIRED_INCLUDES} ${OPUS_INCLUDEDIR}
        )

        CHECK_LIBRARY_EXISTS(ogg ogg_stream_flush_fill
            ${OPUS_LIBDIR} HAVE_OGG_STREAM_FLUSH_FILL
        )
        IF (NOT HAVE_OGG_STREAM_FLUSH_FILL)
            MESSAGE(FATAL_ERROR "
                libogg seems to be too old for use with opus codec,
                it lacks ogg_stream_flush_fill()
            ")
        ENDIF (NOT HAVE_OGG_STREAM_FLUSH_FILL)

        SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES}
            ${OPUS_LIBRARIES}
        )
        SET(HAVE_OGG_OPUS  ON CACHE BOOL "enable Opus codec")
        SET(OGG_OPUS_LIBS "opus")
        SET(OGG_OPUS_SRCS
            OpusCommon.cpp
            OpusDecoder.cpp
            OpusEncoder.cpp
        )
    ENDIF (WITH_OGG_OPUS)

#############################################################################
### check for Vorbis headers and library                                  ###

    IF (WITH_OGG_VORBIS)
        PKG_CHECK_MODULES(VORBIS REQUIRED vorbis>=1.0.0)
        MESSAGE(STATUS "  Found vorbis codec library in ${VORBIS_LIBDIR}")
        MESSAGE(STATUS "  Found vorbis codec headers in ${VORBIS_INCLUDEDIR}")
        CHECK_LIBRARY_EXISTS(vorbis vorbis_bitrate_addblock
            "${VORBIS_LIBDIR}" HAVE_LIBVORBISENC_V2
        )
        IF (NOT HAVE_LIBVORBISENC_V2)
            MESSAGE(FATAL_ERROR "libvorbis lacks vorbis_bitrate_addblock()")
        ENDIF (NOT HAVE_LIBVORBISENC_V2)
        SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES}
            ${VORBIS_LIBRARIES}
        )

        PKG_CHECK_MODULES(VORBISENC REQUIRED vorbisenc>=1.0.0)
        MESSAGE(STATUS "  Found vorbisenc library in ${VORBISENC_LIBDIR}")
        MESSAGE(STATUS "  Found vorbisenc headers in ${VORBISENC_INCLUDEDIR}")
        SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES}
            ${VORBISENC_LIBRARIES}
        )

        CHECK_INCLUDE_FILES(
            "vorbis/codec.h;vorbis/vorbisenc.h"
            HAVE_OGG_VORBIS_HEADERS
        )
        IF (NOT HAVE_OGG_VORBIS_HEADERS)
            MESSAGE(FATAL_ERROR "ogg vorbis header files are missing")
        ENDIF (NOT HAVE_OGG_VORBIS_HEADERS)


        SET(HAVE_OGG_VORBIS ON CACHE BOOL "enable Vorbis codec")
        SET(OGG_VORBIS_LIBS "vorbisenc" "vorbis")
        SET(OGG_VORBIS_SRCS
            VorbisDecoder.cpp
            VorbisEncoder.cpp
        )
    ENDIF (WITH_OGG_VORBIS)

    #############################################################################
    ### common part                                                           ###

    SET(plugin_codec_ogg_LIB_SRCS
        OggCodecPlugin.cpp
        OggDecoder.cpp
        OggEncoder.cpp
        ${OGG_OPUS_SRCS}
        ${OGG_VORBIS_SRCS}
    )

    SET(plugin_codec_ogg_LIBS
        ${OGG_OPUS_LIBS}
        ${OGG_VORBIS_LIBS}
        ogg
    )

    KWAVE_PLUGIN(codec_ogg)

ENDIF (WITH_OGG_OPUS OR WITH_OGG_VORBIS)

#############################################################################
#############################################################################
