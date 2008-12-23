#############################################################################
##    Kwave                - plugins/codec_wav/CMakeLists.txt
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
    WavFormatMap.cpp
    WavPropertyMap.cpp
)

SET(plugin_codec_wav_LIBS audiofile)

KWAVE_PLUGIN(codec_wav)

#############################################################################
#############################################################################
