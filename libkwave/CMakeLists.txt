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

#############################################################################

IF (WITH_OPTIMIZED_MEMCPY)
    SET(libkwave_LIB_SRCS_memcpy cputest.c memcpy.c ppcasm_string.S)
    SET_SOURCE_FILES_PROPERTIES(ppcasm_string.S PROPERTIES LANGUAGE C)
ENDIF (WITH_OPTIMIZED_MEMCPY)

SET(libkwave_LIB_SRCS
    ApplicationContext.cpp
    ClipBoard.cpp
    CodecBase.cpp
    CodecManager.cpp
    CodecPlugin.cpp
    Compression.cpp
    ConfirmCancelProxy.cpp
    Connect.cpp
    Curve.cpp
    Decoder.cpp
    Drag.cpp
    Encoder.cpp
    Filter.cpp
    FileInfo.cpp
    FileProgress.cpp
    Functions.cpp
    GenreType.cpp
    GlobalLock.cpp
    Interpolation.cpp
    Label.cpp
    LabelList.cpp
    MemoryManager.cpp
    MessageBox.cpp
    MetaData.cpp
    MetaDataList.cpp
    MimeData.cpp
    MixerMatrix.cpp
    MultiPlaybackSink.cpp
    MultiStreamWriter.cpp
    MultiTrackReader.cpp
    MultiTrackWriter.cpp
    MultiWriter.cpp
    Parser.cpp
    PlaybackController.cpp
    PlaybackSink.cpp
    PlayBackTypesMap.cpp
    Plugin.cpp
    PluginManager.cpp
    SampleArray.cpp
    SampleSink.cpp
    SampleSource.cpp
    Selection.cpp
    Signal.cpp
    SignalManager.cpp
    SampleEncoderLinear.cpp
    SampleFIFO.cpp
    SampleFormat.cpp
    SampleReader.cpp
    StandardBitrates.cpp
    StreamWriter.cpp
    Stripe.cpp
    SwapFile.cpp
    Track.cpp
    TrackWriter.cpp
    Utils.cpp
    VirtualAudioFile.cpp
    VorbisCommentMap.cpp
    Writer.cpp
    WorkerThread.cpp
    WindowFunction.cpp

    modules/ChannelMixer.cpp
    modules/CurveStreamAdapter.cpp
    modules/Indexer.cpp
    modules/Delay.cpp
    modules/Mul.cpp
    modules/Osc.cpp
    modules/RateConverter.cpp
    modules/SampleBuffer.cpp
    modules/StreamObject.cpp

    undo/UndoAddMetaDataAction.cpp
    undo/UndoDeleteAction.cpp
    undo/UndoDeleteMetaDataAction.cpp
    undo/UndoDeleteTrack.cpp
    undo/UndoInsertAction.cpp
    undo/UndoInsertTrack.cpp
    undo/UndoModifyAction.cpp
    undo/UndoModifyMetaDataAction.cpp
    undo/UndoSelection.cpp
    undo/UndoTransaction.cpp
    undo/UndoTransactionGuard.cpp

    ${libkwave_LIB_SRCS_samplerate}
    ${libkwave_LIB_SRCS_memcpy}
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
