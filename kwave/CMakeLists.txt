#############################################################################
##    Kwave                - kwave/CMakeLists.txt
##                           -------------------
##    begin                : Wed May 09 2007
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

INCLUDE(ECMInstallIcons)

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
    pics/sound_card.png
    pics/sound_device.png
    pics/sound_note.png
    pics/sound_subdevice.png
)

#############################################################################
### scalable action icons to be installed in the icon dir                 ###

SET(kwave_ACTION_ICONS
    toolbar/sc-actions-kwave_player_end.svgz
    toolbar/sc-actions-kwave_player_fwd.svgz
    toolbar/sc-actions-kwave_player_pause.svgz
    toolbar/sc-actions-kwave_player_pause_2.svgz
    toolbar/sc-actions-kwave_player_play.svgz
    toolbar/sc-actions-kwave_player_loop.svgz
    toolbar/sc-actions-kwave_player_record.svgz
    toolbar/sc-actions-kwave_player_rew.svgz
    toolbar/sc-actions-kwave_player_start.svgz
    toolbar/sc-actions-kwave_player_stop.svgz
    toolbar/sc-actions-kwave_viewmagfit.svgz
    toolbar/sc-actions-kwave_viewmag.svgz
    toolbar/sc-actions-kwave_zoom_in.svgz
    toolbar/sc-actions-kwave_zoom_original.svgz
    toolbar/sc-actions-kwave_zoom_out.svgz
)

#############################################################################

ADD_EXECUTABLE(kwave_core ${kwave_SRCS})

#############################################################################

TARGET_LINK_LIBRARIES(kwave_core
    libkwavegui
    libkwave
    audiofile
    ${SAMPLERATE_LIBS}
    Qt5::Core
    Qt5::Widgets
    KF5::Completion
    KF5::ConfigCore
    KF5::Crash
    KF5::DBusAddons
    KF5::I18n
    KF5::IconThemes
    KF5::KIOFileWidgets
    KF5::XmlGui
)

SET_TARGET_PROPERTIES(kwave_core PROPERTIES
    OUTPUT_NAME "kwave"
    ENABLE_EXPORTS TRUE
)

#############################################################################
### install files                                                         ###

# the main program
INSTALL(TARGETS kwave_core     DESTINATION ${BIN_INSTALL_DIR})

# other files
INSTALL(FILES menus.config     DESTINATION ${DATA_INSTALL_DIR}/kwave)
INSTALL(FILES ${kwave_PIXMAPS} DESTINATION ${DATA_INSTALL_DIR}/kwave/pics)
INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kwave.desktop
    DESTINATION ${XDG_APPS_INSTALL_DIR}
)

# action icons
ECM_INSTALL_ICONS(
    ICONS ${kwave_ACTION_ICONS}
    DESTINATION ${ICON_INSTALL_DIR}
)

# application icon
ECM_INSTALL_ICONS(
    ICONS ${CMAKE_CURRENT_SOURCE_DIR}/pics/sc-apps-kwave.svgz
    DESTINATION ${ICON_INSTALL_DIR}
)

# AppData file
INSTALL(
    FILES org.kde.kwave.appdata.xml
    DESTINATION ${KDE_INSTALL_METAINFODIR}
)

#############################################################################
#############################################################################
