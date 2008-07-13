#############################################################################
##    Kwave                - audiofile/CMakeLists.txt
##                           -------------------
##    begin                : Tue May 09 2007
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

#############################################################################

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

#############################################################################

# suppress all libaudiofile warnings, we know that it is not good :-(
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")

ADD_DEFINITIONS( -DHAVE_CONFIG_H )

#############################################################################

SET(libaudiofile_STAT_SRCS
    aes.c
    af_vfs.c
    aiff.c
    aiffwrite.c
    aupv.c
    byteorder.c
    compression.c
    data.c
    error.c
    format.c
    g711.c
    g711.h
    instrument.c
    intfloat_readwrite.c
    ircam.c
    ircam.h
    ircamwrite.c
    loop.c
    marker.c
    misc.c
    next.c
    nextwrite.c
    openclose.c
    pcm.c
    query.c
    raw.c
    setup.c
    track.c
    units.c
    util.c
    wave.c
    wavewrite.c
    modules/adpcm.c
    modules.c
    modules/g711.c
    modules/ima.c
    modules/msadpcm.c
    modules/pcm.c
    modules/rebuffer.c
)

#############################################################################

ADD_LIBRARY(libaudiofile STATIC ${libaudiofile_STAT_SRCS})
LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/modules)
SET_TARGET_PROPERTIES(libaudiofile PROPERTIES OUTPUT_NAME "audiofile")
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

#############################################################################
#############################################################################
