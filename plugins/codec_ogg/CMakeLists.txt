#############################################################################
##    Kwave                - plugins/codec_ogg/CMakeLists.txt
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

OPTION(WITH_OGG "enable support for ogg files [default=on]" ON)
IF (WITH_OGG)

#############################################################################
### check for Ogg/Vorbis headers and library                              ###

    SET(OggVorbis_FIND_QUIETLY TRUE)
    INCLUDE(FindOggVorbis)

    CHECK_INCLUDE_FILES("vorbis/codec.h;vorbis/vorbisenc.h" HAVE_OGG_VORBIS_HEADERS)
    IF (HAVE_OGG_VORBIS_HEADERS)
        SET(CMAKE_REQUIRED_LIBRARIES "vorbisenc;vorbis;ogg")
        IF (OGGVORBIS_VERSION GREATER 1)
            SET(HAVE_OGG_VORBIS TRUE)
        ENDIF (OGGVORBIS_VERSION GREATER 1)
    ENDIF (HAVE_OGG_VORBIS_HEADERS)

    IF (HAVE_OGG_VORBIS)
        MESSAGE(STATUS "Found Ogg/Vorbis library v2.x or newer")
    ELSE (HAVE_OGG_VORBIS)
        MESSAGE(FATAL_ERROR "

        your system lacks support for at least libvorbis-devel!
        please visit "http://vorbis.com" for obtainig a current version
        of libogg and libvorbis plus the -devel packages if your
        distribution does not include them.

        ")
    ENDIF (HAVE_OGG_VORBIS)

    SET(plugin_codec_ogg_LIB_SRCS
        OggCodecPlugin.cpp
        OggDecoder.cpp
        OggEncoder.cpp
    )

    SET(plugin_codec_ogg_LIBS
        vorbisenc vorbis ogg
    )

    KWAVE_PLUGIN(codec_ogg)

ENDIF (WITH_OGG)

#############################################################################
#############################################################################
