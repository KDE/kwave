#############################################################################
##    Kwave                - plugins/playback/CMakeLists.txt
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

IF (HAVE_OSS_SUPPORT)
    SET(PLAYBACK_SOURCES ${PLAYBACK_SOURCES} PlayBack-OSS.cpp PlayBack-OSS.h)
ENDIF (HAVE_OSS_SUPPORT)

IF (HAVE_ALSA_SUPPORT)
    SET(PLAYBACK_SOURCES ${PLAYBACK_SOURCES} PlayBack-ALSA.cpp PlayBack-ALSA.h)
    SET(PLAYBACK_REQUIRED_LIBS ${PLAYBACK_REQUIRED_LIBS} ${ALSA_LIBRARY})
ENDIF (HAVE_ALSA_SUPPORT)

IF (HAVE_PULSEAUDIO_SUPPORT)
    SET(PLAYBACK_SOURCES ${PLAYBACK_SOURCES} PlayBack-PulseAudio.cpp PlayBack-PulseAudio.h)
    SET(PLAYBACK_REQUIRED_LIBS ${PLAYBACK_REQUIRED_LIBS} ${PULSEAUDIO_LINK_LIBRARIES})

    CHECK_INCLUDE_FILES("poll.h" HAVE_POLL_H)
    IF (NOT HAVE_POLL_H)
        MESSAGE(FATAL_ERROR "poll.h is missing (needed for PulseAudio support)")
    ENDIF (NOT HAVE_POLL_H)
ENDIF (HAVE_PULSEAUDIO_SUPPORT)

IF (HAVE_QT_AUDIO_SUPPORT)
    SET(PLAYBACK_SOURCES ${PLAYBACK_SOURCES} PlayBack-Qt.cpp PlayBack-Qt.h)
    SET(PLAYBACK_REQUIRED_LIBS ${PLAYBACK_REQUIRED_LIBS} Qt::Multimedia)
ENDIF (HAVE_QT_AUDIO_SUPPORT)

SET(plugin_playback_LIB_SRCS
    PlayBackDialog.cpp
    PlayBackPlugin.cpp
    PlayBackDialog.h
    PlayBackPlugin.h
    ${PLAYBACK_SOURCES}
)

SET(plugin_playback_LIB_UI
    PlayBackDlg.ui
)

IF (PLAYBACK_REQUIRED_LIBS)
    SET(plugin_playback_LIBS ${PLAYBACK_REQUIRED_LIBS})
ENDIF (PLAYBACK_REQUIRED_LIBS)

KWAVE_PLUGIN(playback)

#############################################################################
#############################################################################
