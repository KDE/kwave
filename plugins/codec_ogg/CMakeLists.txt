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

OPTION(WITH_OGG_VORBIS "enable support for Ogg/Vorbis files [default=on]" ON)
OPTION(WITH_OGG_OPUS   "enable support for Ogg/Opus files [default=on]" ON)

SET(WITH_OGG OFF)
IF (WITH_OGG_OPUS OR WITH_OGG_VORBIS)
    SET(WITH_OGG ON)
ENDIF (WITH_OGG_OPUS OR WITH_OGG_VORBIS)

IF (WITH_OGG)

#############################################################################
### check for Ogg/Opus headers and library                                ###

    IF (WITH_OGG_OPUS)
	PKG_CHECK_MODULES(OPUS REQUIRED opus>=1.0.0)
	IF (NOT OPUS_FOUND)
	    MESSAGE(FATAL_ERROR "Opus codec not found")
	ENDIF(NOT OPUS_FOUND)

	MESSAGE(STATUS "Found Opus codec version ${OPUS_VERSION}")
# 	MESSAGE(STATUS "Found Opus codec library in ${OPUS_LIBDIR}")
# 	MESSAGE(STATUS "Found Opus codec headers in ${OPUS_INCLUDEDIR}")

	CHECK_LIBRARY_EXISTS(ogg ogg_stream_flush_fill "" HAVE_OGG_STREAM_FLUSH_FILL)

	SET(HAVE_OGG_OPUS  ON CACHE BOOL "enable Opus codec")
	SET(OGG_OPUS_LIBS "opus")
	SET(OGG_OPUS_SRCS
	    OpusCommon.cpp
	    OpusDecoder.cpp
	    OpusEncoder.cpp
	)
    ENDIF (WITH_OGG_OPUS)

#############################################################################
### check for Ogg/Vorbis headers and library                              ###

    IF (WITH_OGG_VORBIS)
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
	    please visit \"http://vorbis.com\" for obtainig a current version
	    of libogg and libvorbis plus the -devel packages if your
	    distribution does not include them.
	    ")
	ENDIF (HAVE_OGG_VORBIS)

	SET(HAVE_OGG_VORBIS ON CACHE BOOL "enable Vorbis codec")
	SET(OGG_VORBIS_LIBS vorbisenc vorbis)
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

ENDIF (WITH_OGG)

#############################################################################
#############################################################################
