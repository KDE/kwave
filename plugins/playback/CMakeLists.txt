#############################################################################
##    Kwave                - plugins/playback/CMakeLists.txt
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

IF (HAVE_ARTS_SUPPORT)
    SET(PLAYBACK_SOURCES ${PLAYBACK_SOURCES} PlayBack-aRts.cpp)
ENDIF (HAVE_ARTS_SUPPORT)
IF (HAVE_OSS_SUPPORT)
    SET(PLAYBACK_SOURCES ${PLAYBACK_SOURCES} PlayBack-OSS.cpp)
ENDIF (HAVE_OSS_SUPPORT)
IF (HAVE_ALSA_SUPPORT)
    SET(PLAYBACK_SOURCES ${PLAYBACK_SOURCES} PlayBack-ALSA.cpp)
    SET(PLAYBACK_REQUIRED_LIBS ${PLAYBACK_REQUIRED_LIBS} asound)
ENDIF (HAVE_ALSA_SUPPORT)

SET(plugin_playback_LIB_SRCS
    PlayBackDialog.cpp
    PlayBackPlugin.cpp
    PlayBackTypesMap.cpp
    SampleEncoderLinear.cpp
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
