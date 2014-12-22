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

# check for programs needed by the script
FIND_REQUIRED_PROGRAM(CAT_EXECUTABLE cat)

SET(MENUSCONFIG2CPP ${CMAKE_SOURCE_DIR}/bin/menusconfig2cpp.pl)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/menus_config_i18n.cpp
    COMMAND ${MENUSCONFIG2CPP}
        ${CMAKE_CURRENT_SOURCE_DIR}/menus.config
        ${CMAKE_CURRENT_BINARY_DIR}/menus_config_i18n.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/menus.config
    DEPENDS ${MENUSCONFIG2CPP}
)

#############################################################################

SET(kwave_SRCS
    App.cpp
    FileContext.cpp
    main.cpp
    MainWidget.cpp
    PlayerToolBar.cpp
    ShortcutWrapper.cpp
    Splash.cpp
    TopWidget.cpp
    ZoomToolBar.cpp
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
    pics/selectedknob.xpm
    pics/sound_device.png
    pics/sound_subdevice.png
    pics/sound_card.png
    pics/sound_note.png
)

#############################################################################
### scalable action icons to be installed in the icon dir                 ###

SET(kwave_ACTION_ICONS
    toolbar/kwave_player_end.svgz
    toolbar/kwave_player_fwd.svgz
    toolbar/kwave_player_pause.svgz
    toolbar/kwave_player_pause_2.svgz
    toolbar/kwave_player_play.svgz
    toolbar/kwave_player_loop.svgz
    toolbar/kwave_player_record.svgz
    toolbar/kwave_player_rew.svgz
    toolbar/kwave_player_start.svgz
    toolbar/kwave_player_stop.svgz
    toolbar/kwave_viewmagfit.svgz
    toolbar/kwave_viewmag.svgz
    toolbar/kwave_zoom_in.svgz
    toolbar/kwave_zoom_original.svgz
    toolbar/kwave_zoom_out.svgz
)

#############################################################################

QT4_AUTOMOC( ${kwave_SRCS} )
KDE4_ADD_EXECUTABLE(kwave_core ${kwave_SRCS})

#############################################################################



TARGET_LINK_LIBRARIES(kwave_core
    kwavegui
    kwave
    audiofile
    ${SAMPLERATE_LIBS}
    ${QT_AND_KDECORE_LIBS}
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

# action icons
INSTALL(
    FILES ${kwave_ACTION_ICONS}
    DESTINATION ${DESTDIR}${KDE4_ICON_INSTALL_DIR}/hicolor/scalable/actions
)

# application icon
INSTALL(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/pics/kwave.svgz
    DESTINATION ${DESTDIR}${KDE4_ICON_INSTALL_DIR}/hicolor/scalable/apps
)

#############################################################################
#############################################################################
