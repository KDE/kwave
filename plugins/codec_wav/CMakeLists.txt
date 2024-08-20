#############################################################################
##    Kwave                - plugins/codec_wav/CMakeLists.txt
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

SET(plugin_codec_wav_LIB_SRCS
    RecoveryBuffer.cpp
    RecoveryMapping.cpp
    RecoverySource.cpp
    RepairVirtualAudioFile.cpp
    RIFFChunk.cpp
    RIFFParser.cpp
    WavCodecPlugin.cpp
    WavDecoder.cpp
    WavEncoder.cpp
    WavFileFormat.cpp
    WavFormatMap.cpp
    WavPropertyMap.cpp

    RecoveryBuffer.h
    RecoveryMapping.h
    RecoverySource.h
    RepairVirtualAudioFile.h
    RIFFChunk.h
    RIFFParser.h
    WavCodecPlugin.h
    WavDecoder.h
    WavEncoder.h
    WavFileFormat.h
    WavFormatMap.h
    WavPropertyMap.h
)

SET(plugin_codec_wav_LIBS audiofile)

KWAVE_PLUGIN(codec_wav)

#############################################################################
#############################################################################
