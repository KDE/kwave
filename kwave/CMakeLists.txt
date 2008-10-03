#############################################################################
##    Kwave                - kwave/CMakeLists.txt
##                           -------------------
##    begin                : Wed May 09 2007
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
### generate menus_config_i18n.cpp                                        ###

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/menus_config_i18n.cpp
    COMMAND ${PROJECT_SOURCE_DIR}/bin/menusconfig2cpp.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/menus.config
        ${CMAKE_CURRENT_BINARY_DIR}/menus_config_i18n.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/menus.config
    DEPENDS ${PROJECT_SOURCE_DIR}/bin/menusconfig2cpp.sh
)

#############################################################################

SET(kwave_SRCS
    main.cpp
    KwaveApp.cpp
    KwaveSplash.cpp
    MainWidget.cpp
    ShortcutWrapper.cpp
    TopWidget.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/menus_config_i18n.cpp
)

#############################################################################
### some pixmaps to be installed in the data dir                          ###

SET(kwave_PIXMAPS
    pics/amplify_free.png
    pics/fade_in.png
    pics/fade_out.png
    pics/knob.xpm
    pics/kwave-splash.png
    pics/light_off.xpm
    pics/light_on.xpm
    pics/logo.xpm
    pics/noise.png
    pics/playback_loop.xpm
    pics/playback_start.xpm
    pics/playback_stop.xpm
    pics/playback_pause.xpm
    pics/playback_pause2.xpm
    pics/selectedknob.xpm
    pics/sound_device.png
    pics/sound_subdevice.png
    pics/sound_card.png
    pics/sound_note.png
    toolbar/zoomall.xpm
    toolbar/zoomin.xpm
    toolbar/zoomnormal.xpm
    toolbar/zoomout.xpm
    toolbar/zoomrange.xpm
)

#############################################################################

QT4_AUTOMOC( ${kwave_SRCS} )
KDE4_ADD_EXECUTABLE(kwave_core ${kwave_SRCS})

#############################################################################

IF (USE_SYSTEM_LIB_AUDIOFILE)
    SET(AUDIOFILE_LIBS audiofile)
ENDIF (USE_SYSTEM_LIB_AUDIOFILE)

TARGET_LINK_LIBRARIES(kwave_core
    kwavegui
    kwave
    ${QT_AND_KDECORE_LIBS}
    ${AUDIOFILE_LIBS}
)

ADD_DEPENDENCIES(kwave_core libkwavegui libkwave)

SET_TARGET_PROPERTIES(kwave_core PROPERTIES OUTPUT_NAME "kwave")

#############################################################################
### install files                                                         ###

# the main program
INSTALL(TARGETS kwave_core     DESTINATION ${DESTDIR}${KDE4_BIN_INSTALL_DIR})

# other files
INSTALL(FILES menus.config     DESTINATION ${DESTDIR}${KDE4_DATA_INSTALL_DIR}/kwave)
INSTALL(FILES ${kwave_PIXMAPS} DESTINATION ${DESTDIR}${KDE4_DATA_INSTALL_DIR}/kwave/pics)
INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kwave.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)

# application icon
INSTALL(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/pics/kwave.svgz
    DESTINATION ${DESTDIR}${KDE4_ICON_INSTALL_DIR}/hicolor/scalable/apps
)

#############################################################################
#############################################################################
