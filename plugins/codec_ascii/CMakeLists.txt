#############################################################################
##    Kwave                - plugins/codec_ascii/CMakeLists.txt
##                           -------------------
##    begin                : Mon May 28 2007
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

SET(plugin_codec_ascii_LIB_SRCS
    AsciiCodecPlugin.cpp
    AsciiDecoder.cpp
    AsciiEncoder.cpp
)

KWAVE_PLUGIN(codec_ascii)

#############################################################################
#############################################################################
