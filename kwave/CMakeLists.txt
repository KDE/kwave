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
    Qt::Core
    Qt::Widgets
    KF6::Completion
    KF6::ConfigCore
    KF6::Crash
    KF6::DBusAddons
    KF6::I18n
    KF6::IconThemes
    KF6::KIOFileWidgets
    KF6::XmlGui
)

SET_TARGET_PROPERTIES(kwave_core PROPERTIES
    OUTPUT_NAME "kwave"
    ENABLE_EXPORTS TRUE
)

#############################################################################
### install files                                                         ###

# the main program
INSTALL(TARGETS kwave_core     DESTINATION ${KDE_INSTALL_BINDIR})

# other files
INSTALL(FILES menus.config     DESTINATION ${KDE_INSTALL_DATADIR}/kwave)
INSTALL(FILES ${kwave_PIXMAPS} DESTINATION ${KDE_INSTALL_DATADIR}/kwave/pics)
INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kwave.desktop
    DESTINATION ${KDE_INSTALL_APPDIR}
)

# action icons
ECM_INSTALL_ICONS(
    ICONS ${kwave_ACTION_ICONS}
    DESTINATION ${KDE_INSTALL_ICONDIR}
)

# application icon
ECM_INSTALL_ICONS(
    ICONS ${CMAKE_CURRENT_SOURCE_DIR}/pics/sc-apps-kwave.svgz
    DESTINATION ${KDE_INSTALL_ICONDIR}
)

# AppData file
INSTALL(
    FILES org.kde.kwave.appdata.xml
    DESTINATION ${KDE_INSTALL_METAINFODIR}
)

#############################################################################
#############################################################################
