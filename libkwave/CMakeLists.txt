#############################################################################
##    Kwave                - libkwave/CMakeLists.txt
##                           -------------------
##    begin                : Tue May 01 2007
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

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ppcasm_string.o
    COMMAND ${CMAKE_C_COMPILER} -I${CMAKE_BINARY_DIR} -Wall -O2
        -DHAVE_CONFIG_H
        -o ${CMAKE_CURRENT_BINARY_DIR}/ppcasm_string.o
        -c ${CMAKE_CURRENT_SOURCE_DIR}/ppcasm_string.S
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ppcasm_string.S
)

#############################################################################

SET(libkwave_LIB_SRCS
    CodecBase.cpp
    CompressionType.cpp
    cputest.c
    Curve.cpp
    CurveStreamAdapter.cpp
    Decoder.cpp
    Encoder.cpp
    Filter.cpp
    FileInfo.cpp
    FileFormat.cpp
    Functions.cpp
    GenreType.cpp
    Interpolation.cpp
    KwaveConnect.cpp
    KwaveDelay.cpp
    KwaveDrag.cpp
    KwaveFilterPlugin.cpp
    KwaveMul.cpp
    KwaveMultiPlaybackSink.cpp
    KwaveOsc.cpp
    KwavePlaybackSink.cpp
    KwavePlugin.cpp
    KwaveSampleArray.cpp
    KwaveSampleSink.cpp
    KwaveSampleSource.cpp
    KwaveStreamObject.cpp
    Label.cpp
    LabelList.cpp
    memcpy.c
    MultiTrackReader.cpp
    MultiTrackWriter.cpp
    Parser.cpp
    ppcasm_string.S
    Selection.cpp
    Signal.cpp
    SampleFIFO.cpp
    SampleFormat.cpp
    SampleReader.cpp
    SampleWriter.cpp
    StandardBitrates.cpp
    Stripe.cpp
    Track.cpp
    VirtualAudioFile.cpp
    WindowFunction.cpp
)

#############################################################################

QT4_AUTOMOC(${libkwave_LIB_SRCS})
KDE4_ADD_LIBRARY(libkwave SHARED ${libkwave_LIB_SRCS})

ADD_DEPENDENCIES(libkwave libkwavemt)

IF (NOT USE_SYSTEM_LIB_AUDIOFILE)
TARGET_LINK_LIBRARIES(libkwave audiofile)
ADD_DEPENDENCIES(libkwave audiofile)
ENDIF (NOT USE_SYSTEM_LIB_AUDIOFILE)

TARGET_LINK_LIBRARIES(libkwave audiofile ${QT_AND_KDECORE_LIBS})

#############################################################################

SET_TARGET_PROPERTIES(libkwave PROPERTIES OUTPUT_NAME "kwave")
SET_TARGET_PROPERTIES(libkwave PROPERTIES LINK_FLAGS
    "-Wl,--whole-archive -lkwavemt -Wl,-no-whole-archive -Wl,--export-dynamic")
SET_TARGET_PROPERTIES(libkwave PROPERTIES
    VERSION ${KWAVE_SOVERSION} SOVERSION ${KWAVE_VERSION_MAJOR} )
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

#############################################################################
### install files                                                         ###

INSTALL(TARGETS libkwave LIBRARY DESTINATION ${LIB_INSTALL_DIR})

#############################################################################
#############################################################################
