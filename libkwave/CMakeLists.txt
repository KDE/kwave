#############################################################################
##    Kwave                - libkwave/CMakeLists.txt
##                           -------------------
##    begin                : Tue May 01 2007
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

#############################################################################

SET(libkwave_LIB_SRCS
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
    Logger.cpp
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
    Track.cpp
    TrackWriter.cpp
    Utils.cpp
    VirtualAudioFile.cpp
    VorbisCommentMap.cpp
    Writer.cpp
    WorkerThread.cpp
    WindowFunction.cpp

    ClipBoard.h
    CodecBase.h
    CodecManager.h
    CodecPlugin.h
    Compression.h
    ConfirmCancelProxy.h
    Connect.h
    Curve.h
    Decoder.h
    Drag.h
    Encoder.h
    Filter.h
    FileInfo.h
    FileProgress.h
    Functions.h
    GenreType.h
    GlobalLock.h
    Interpolation.h
    Label.h
    LabelList.h
    Logger.h
    MessageBox.h
    MetaData.h
    MetaDataList.h
    MimeData.h
    MixerMatrix.h
    MultiPlaybackSink.h
    MultiStreamWriter.h
    MultiTrackReader.h
    MultiTrackWriter.h
    MultiWriter.h
    Parser.h
    PlaybackController.h
    PlaybackSink.h
    PlayBackTypesMap.h
    Plugin.h
    PluginManager.h
    SampleArray.h
    SampleSink.h
    SampleSource.h
    Selection.h
    Signal.h
    SignalManager.h
    SampleEncoderLinear.h
    SampleFIFO.h
    SampleFormat.h
    SampleReader.h
    StandardBitrates.h
    StreamWriter.h
    Stripe.h
    Track.h
    TrackWriter.h
    Utils.h
    VirtualAudioFile.h
    VorbisCommentMap.h
    Writer.h
    WorkerThread.h
    WindowFunction.h

    modules/ChannelMixer.cpp
    modules/CurveStreamAdapter.cpp
    modules/Indexer.cpp
    modules/Delay.cpp
    modules/Mul.cpp
    modules/Osc.cpp
    modules/RateConverter.cpp
    modules/SampleBuffer.cpp
    modules/StreamObject.cpp

    modules/ChannelMixer.h
    modules/CurveStreamAdapter.h
    modules/Indexer.h
    modules/Delay.h
    modules/Mul.h
    modules/Osc.h
    modules/RateConverter.h
    modules/SampleBuffer.h
    modules/StreamObject.h

    undo/UndoAddMetaDataAction.cpp
    undo/UndoDeleteAction.cpp
    undo/UndoDeleteMetaDataAction.cpp
    undo/UndoDeleteTrack.cpp
    undo/UndoInsertAction.cpp
    undo/UndoInsertTrack.cpp
    undo/UndoManager.cpp
    undo/UndoModifyAction.cpp
    undo/UndoModifyMetaDataAction.cpp
    undo/UndoSelection.cpp
    undo/UndoTransaction.cpp
    undo/UndoTransactionGuard.cpp

    undo/UndoAddMetaDataAction.h
    undo/UndoDeleteAction.h
    undo/UndoDeleteMetaDataAction.h
    undo/UndoDeleteTrack.h
    undo/UndoInsertAction.h
    undo/UndoInsertTrack.h
    undo/UndoManager.h
    undo/UndoModifyAction.h
    undo/UndoModifyMetaDataAction.h
    undo/UndoSelection.h
    undo/UndoTransaction.h
    undo/UndoTransactionGuard.h

    ${libkwave_LIB_SRCS_samplerate}
)

#############################################################################

ADD_LIBRARY(libkwave SHARED ${libkwave_LIB_SRCS})

TARGET_LINK_LIBRARIES(libkwave
    ${LIBAUDIOFILE_LINK_LIBRARIES}
    ${SAMPLERATE_LINK_LIBRARIES}
    Qt::Core
    Qt::Concurrent
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::WidgetsAddons
    KF6::XmlGui
    pthread
)

#############################################################################

SET_TARGET_PROPERTIES(libkwave PROPERTIES OUTPUT_NAME "kwave")
SET_TARGET_PROPERTIES(libkwave PROPERTIES LINK_FLAGS "-Wl,--export-dynamic")
SET_TARGET_PROPERTIES(libkwave PROPERTIES
    VERSION ${KWAVE_VERSION} SOVERSION ${KWAVE_VERSION_MAJOR} )
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

ECM_GENERATE_EXPORT_HEADER(libkwave
    VERSION ${KWAVE_VERSION}
)

#############################################################################
### install files                                                         ###

INSTALL(TARGETS libkwave LIBRARY DESTINATION ${KDE_INSTALL_LIBDIR} NAMELINK_SKIP)

#############################################################################
#############################################################################

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
