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

IF (WITH_SAMPLERATE)
    SET(libkwave_LIB_SRCS_samplerate modules/RateConverter.cpp)
ENDIF (WITH_SAMPLERATE)

SET(libkwave_LIB_SRCS
    ApplicationContext.cpp
    ClipBoard.cpp
    CodecBase.cpp
    CodecManager.cpp
    CompressionType.cpp
    ConfirmCancelProxy.cpp
    cputest.c
    Curve.cpp
    Decoder.cpp
    Encoder.cpp
    Filter.cpp
    FileInfo.cpp
    FileFormat.cpp
    FileProgress.cpp
    Functions.cpp
    GenreType.cpp
    Interpolation.cpp
    KwaveConnect.cpp
    KwaveDrag.cpp
    KwaveMimeData.cpp
    KwaveMultiPlaybackSink.cpp
    KwavePlaybackSink.cpp
    KwavePlugin.cpp
    KwaveSampleArray.cpp
    KwaveSampleSink.cpp
    KwaveSampleSource.cpp
    Label.cpp
    LabelList.cpp
    memcpy.c
    MemoryManager.cpp
    MessageBox.cpp
    MetaData.cpp
    MultiWriter.cpp
    MultiStreamWriter.cpp
    MultiTrackReader.cpp
    MultiTrackWriter.cpp
    Parser.cpp
    PlaybackController.cpp
    PluginManager.cpp
    PluginWorkerThread.cpp
    ppcasm_string.S
    Selection.cpp
    Signal.cpp
    SignalManager.cpp
    SampleFIFO.cpp
    SampleFormat.cpp
    SampleReader.cpp
    StandardBitrates.cpp
    StreamWriter.cpp
    Stripe.cpp
    SwapFile.cpp
    Track.cpp
    TrackWriter.cpp
    VirtualAudioFile.cpp
    Writer.cpp
    WindowFunction.cpp

    modules/CurveStreamAdapter.cpp
    modules/KwaveDelay.cpp
    modules/KwaveMul.cpp
    modules/KwaveOsc.cpp
    modules/KwaveStreamObject.cpp

    undo/UndoAddLabelAction.cpp
    undo/UndoDeleteAction.cpp
    undo/UndoDeleteLabelAction.cpp
    undo/UndoDeleteTrack.cpp
    undo/UndoFileInfo.cpp
    undo/UndoInsertAction.cpp
    undo/UndoInsertTrack.cpp
    undo/UndoModifyAction.cpp
    undo/UndoModifyLabelAction.cpp
    undo/UndoSelection.cpp
    undo/UndoTransaction.cpp
    undo/UndoTransactionGuard.cpp

    ${libkwave_LIB_SRCS_samplerate}
)

#############################################################################

QT4_AUTOMOC(${libkwave_LIB_SRCS})
KDE4_ADD_LIBRARY(libkwave SHARED ${libkwave_LIB_SRCS})

TARGET_LINK_LIBRARIES(libkwave
    audiofile
    ${SAMPLERATE_LIBS}
    ${QT_AND_KDECORE_LIBS}
)

#############################################################################

SET_TARGET_PROPERTIES(libkwave PROPERTIES OUTPUT_NAME "kwave")
SET_TARGET_PROPERTIES(libkwave PROPERTIES LINK_FLAGS "-Wl,--export-dynamic")
SET_TARGET_PROPERTIES(libkwave PROPERTIES
    VERSION ${KWAVE_SOVERSION} SOVERSION ${KWAVE_VERSION_MAJOR} )
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

#############################################################################
### install files                                                         ###

INSTALL(TARGETS libkwave LIBRARY DESTINATION ${KDE4_LIB_INSTALL_DIR} NAMELINK_SKIP)


#############################################################################
#############################################################################
