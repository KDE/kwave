#############################################################################
##    Kwave                - plugins/codec_mp3/CMakeLists.txt
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

OPTION(WITH_MP3 "enable support for mp3 files [default=on]" ON)
IF (WITH_MP3)

#############################################################################
### check for id3lib headers and library                                  ###

    INCLUDE(CheckIncludeFileCXX)

    find_path(ID3LIB_INCLUDES id3/globals.h)
    SET(CMAKE_REQUIRED_INCLUDES ${ID3LIB_INCLUDES})

    CHECK_INCLUDE_FILE_CXX(id3/globals.h HAVE_ID3_HEADER_GLOBALS_H)
    IF (HAVE_ID3_HEADER_GLOBALS_H)
        CHECK_INCLUDE_FILE_CXX(id3/tag.h HAVE_ID3_HEADER_TAG_H)
    ENDIF (HAVE_ID3_HEADER_GLOBALS_H)
    IF (HAVE_ID3_HEADER_TAG_H)
        CHECK_INCLUDE_FILE_CXX(id3/misc_support.h HAVE_ID3_HEADERS)
    ENDIF (HAVE_ID3_HEADER_TAG_H)

    IF (HAVE_ID3_HEADERS)
        SET(_try_src_dir "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp")
        SET(_try_id3lib "${_try_src_dir}/cmake_try_id3lib.cpp")
        WRITE_FILE("${_try_id3lib}" "
            #include <id3/globals.h>
            #include <id3/readers.h>
            #include <id3/tag.h>
            int main()
            {
                ID3_Tag tag;
                ID3_MemoryReader reader;
                tag.Link(reader);
                return 0;
            }
        ")
        TRY_COMPILE(
            HAVE_ID3_LIB
            ${CMAKE_BINARY_DIR}
            ${_try_id3lib}
            CMAKE_FLAGS
                 "-DINCLUDE_DIRECTORIES=${ID3LIB_INCLUDES}"
                 "-DLINK_DIRECTORIES=${CMAKE_INSTALL_PREFIX}/lib"
                 "-DLINK_LIBRARIES:STRING=id3\;stdc++\;z"
            OUTPUT_VARIABLE _out
        )
    ENDIF (HAVE_ID3_HEADERS)

    IF (NOT HAVE_ID3_LIB)
        MESSAGE(FATAL_ERROR "

        id3lib seems not to be installed or too old !

        You can download id3lib from http://id3lib.sourceforge.net/
        and fetch the latest version from there. If you download the
        rpm package, please don't forget to fetch id3lib-devel too,
        it is needed for compiling Kwave!

        Note: at least version 3.8.1 is needed.
        ")
    ENDIF (NOT HAVE_ID3_LIB)

#############################################################################
### check for libmad headers and library                                  ###

    find_path(LIBMAD_INCLUDES mad.h)
    SET(CMAKE_REQUIRED_INCLUDES ${LIBMAD_INCLUDES})
    CHECK_INCLUDE_FILES(mad.h HAVE_MAD_H)
    IF (HAVE_MAD_H)
        CHECK_LIBRARY_EXISTS(mad mad_decoder_init
            "${CMAKE_INSTALL_PREFIX}/lib" HAVE_MAD_LIB)
    ENDIF (HAVE_MAD_H)

    IF (NOT HAVE_MAD_LIB)
        MESSAGE(FATAL_ERROR "

        the mad package is missing.
        please install the mad and the mad-devel package!

        ")
    ENDIF (NOT HAVE_MAD_LIB)

#############################################################################

    SET(HAVE_MP3  ON CACHE BOOL "enable MP3 codec")

    SET(plugin_codec_mp3_LIB_SRCS
        ID3_PropertyMap.cpp
        ID3_QIODeviceReader.cpp
        ID3_QIODeviceWriter.cpp
        MP3CodecPlugin.cpp
        MP3Decoder.cpp
        MP3Encoder.cpp
        MP3EncoderDialog.cpp
        MP3EncoderSettings.cpp

        ID3_PropertyMap.h
        ID3_QIODeviceReader.h
        ID3_QIODeviceWriter.h
        MP3CodecPlugin.h
        MP3Decoder.h
        MP3Encoder.h
        MP3EncoderDialog.h
        MP3EncoderSettings.h
    )

    SET(plugin_codec_mp3_LIB_UI
        MP3EncoderDialogBase.ui
    )

    SET(plugin_codec_mp3_LIBS
        mad id3 stdc++ z
    )

    KWAVE_PLUGIN(codec_mp3)

ENDIF (WITH_MP3)

#############################################################################
#############################################################################
