#############################################################################
##    Kwave                - plugins/record/CMakeLists.txt
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

#############################################################################

IF (HAVE_OSS_SUPPORT)
    SET(RECORD_SOURCES ${RECORD_SOURCES} Record-OSS.cpp)
ENDIF (HAVE_OSS_SUPPORT)

IF (HAVE_ALSA_SUPPORT)
    SET(RECORD_SOURCES ${RECORD_SOURCES} Record-ALSA.cpp)
    SET(RECORD_REQUIRED_LIBS ${RECORD_REQUIRED_LIBS} asound)
ENDIF (HAVE_ALSA_SUPPORT)

IF (HAVE_PULSEAUDIO_SUPPORT)
    SET(RECORD_SOURCES ${RECORD_SOURCES} Record-PulseAudio.cpp)
    SET(RECORD_REQUIRED_LIBS ${RECORD_REQUIRED_LIBS} ${PULSEAUDIO_LIBRARIES})

    CHECK_INCLUDE_FILES("poll.h" HAVE_POLL_H)
    IF (NOT HAVE_POLL_H)
        MESSAGE(FATAL_ERROR "poll.h is missing (needed for PulseAudio support)")
    ENDIF (NOT HAVE_POLL_H)
ENDIF (HAVE_PULSEAUDIO_SUPPORT)

IF (HAVE_QT_AUDIO_SUPPORT)
    SET(RECORD_SOURCES ${RECORD_SOURCES} Record-Qt.cpp)
    SET(RECORD_REQUIRED_LIBS ${RECORD_REQUIRED_LIBS} Qt::Multimedia)
ENDIF (HAVE_QT_AUDIO_SUPPORT)

SET(plugin_record_LIB_SRCS
    LevelMeter.cpp
    RecordController.cpp
    RecordDialog.cpp
    RecordParams.cpp
    RecordPlugin.cpp
    RecordThread.cpp
    RecordTypesMap.cpp
    SampleDecoderLinear.cpp
    StatusWidget.cpp
    ${RECORD_SOURCES}
)

SET(plugin_record_LIB_UI
    RecordDlg.ui
)

IF (RECORD_REQUIRED_LIBS)
    SET(plugin_record_LIBS ${RECORD_REQUIRED_LIBS})
ENDIF (RECORD_REQUIRED_LIBS)

KWAVE_PLUGIN(record)

#############################################################################
#############################################################################
