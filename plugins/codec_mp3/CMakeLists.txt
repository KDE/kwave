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
    INCLUDE(FindPkgConfig)
    INCLUDE(UsePkgConfig)

    PKG_CHECK_MODULES(MADLIB REQUIRED mad>=0.15.0)
    PKG_CHECK_MODULES(TAGLIB REQUIRED taglib>=2.0)

    ### TODO legacy ###
    find_library(ID3LIB NAMES id3 REQUIRED)

    SET(HAVE_MP3  ON CACHE BOOL "enable MP3 codec")

    SET(plugin_codec_mp3_LIB_SRCS
        ID3_PropertyMap.cpp
        ID3_QIODeviceReader.cpp ### TODO legacy ###
        ID3_QIODeviceWriter.cpp ### TODO legacy ###
        MP3CodecPlugin.cpp
        MP3Decoder.cpp
        MP3Encoder.cpp
        MP3EncoderDialog.cpp
        MP3EncoderSettings.cpp
        TagLib_PropertyMap.cpp

        ID3_PropertyMap.h
        ID3_QIODeviceReader.h ### TODO legacy ###
        ID3_QIODeviceWriter.h ### TODO legacy ###
        MP3CodecPlugin.h
        MP3Decoder.h
        MP3Encoder.h
        MP3EncoderDialog.h
        MP3EncoderSettings.h
        TagLib_PropertyMap.h
        TagLib_QIODeviceStream.h
    )

    SET(plugin_codec_mp3_LIB_UI
        MP3EncoderDialogBase.ui
    )

    SET(plugin_codec_mp3_LIBS
        ${MADLIB_LIBRARIES} ${ID3LIB} ${TAGLIB_LIBRARIES}
    )

    ### TODO legacy ###
    INCLUDE(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-Wno-non-virtual-dtor" HAVE_WNO_NON_VIRTUAL_DTOR)
    IF (HAVE_WNO_NON_VIRTUAL_DTOR)
        SET_SOURCE_FILES_PROPERTIES(
            MP3Decoder.cpp
            MP3Encoder.cpp
            ID3_QIODeviceReader.cpp
            ID3_QIODeviceWriter.cpp
            PROPERTIES COMPILE_FLAGS "-Wno-non-virtual-dtor"
        )
    ENDIF (HAVE_WNO_NON_VIRTUAL_DTOR)

    KWAVE_PLUGIN(codec_mp3)

ENDIF (WITH_MP3)

#############################################################################
#############################################################################
