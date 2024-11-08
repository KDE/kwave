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

    find_library(ID3LIB NAMES id3 REQUIRED)
    find_library(LIBMAD NAMES mad REQUIRED)

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
        ${LIBMAD} ${ID3LIB} stdc++ z
    )

    KWAVE_PLUGIN(codec_mp3)

ENDIF (WITH_MP3)

#############################################################################
#############################################################################
