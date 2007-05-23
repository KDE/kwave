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

FIND_PACKAGE(RequiredProgram REQUIRED)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE3_INCLUDE_DIR}
    ${QT_INCLUDE_DIR}
)

FIND_REQUIRED_PROGRAM(CONVERT_EXECUTABLE convert)

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
    ClipBoard.cpp
    CodecManager.cpp
    KwaveApp.cpp
    KwaveSplash.cpp
    LabelManager.cpp
    MainWidget.cpp
    MemoryManager.cpp
    MouseMark.cpp
    PlaybackController.cpp
    PluginManager.cpp
    SignalManager.cpp
    SignalWidget.cpp
    SwapFile.cpp
    TopWidget.cpp
    UndoAddLabelAction.cpp
    UndoDeleteAction.cpp
    UndoDeleteLabelAction.cpp
    UndoDeleteTrack.cpp
    UndoFileInfo.cpp
    UndoInsertAction.cpp
    UndoInsertTrack.cpp
    UndoModifyAction.cpp
    UndoModifyLabelAction.cpp
    UndoSelection.cpp
    UndoTransaction.cpp
    UndoTransactionGuard.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/menus_config_i18n.cpp
)

#############################################################################
### macro for scaling icons                                               ###

MACRO(KWAVE_SCALE_ICONS _sources)
    FOREACH(_size ${ARGN})
        SET(_orig ${CMAKE_CURRENT_SOURCE_DIR}/pics/kwave.xpm)
        SET(_scaled ${CMAKE_CURRENT_BINARY_DIR}/kwave-${_size}.png)

        # generate the scaled version
        ADD_CUSTOM_COMMAND(OUTPUT ${_scaled}
            COMMAND ${CONVERT_EXECUTABLE}
                -geometry ${_size} ${_orig} ${_scaled}
            DEPENDS ${_orig}
        )
        SET(${_sources} ${${_sources}} ${_scaled})
        SET_SOURCE_FILES_PROPERTIES(${_scaled} PROPERTIES GENERATED true)
        SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
            "${_scaled}")
    ENDFOREACH(_size)
ENDMACRO(KWAVE_SCALE_ICONS)

#############################################################################
### scaled icons                                                          ###

KWAVE_SCALE_ICONS(kwave_SRCS
    16x16
    22x22
    32x32
    48x48
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
    pics/lspeaker.xpm
    pics/minifont.xpm
    pics/noise.png
    pics/playback_loop.xpm
    pics/playback_start.xpm
    pics/playback_stop.xpm
    pics/playback_pause.xpm
    pics/playback_pause2.xpm
    pics/rspeaker.xpm
    pics/sound_arts.png
    pics/sound_device.png
    pics/sound_subdevice.png
    pics/sound_card.png
    pics/sound_note.png
    pics/xspeaker.xpm
)

#############################################################################

KDE3_AUTOMOC(${kwave_SRCS})
KDE3_ADD_EXECUTABLE(kwave_core ${kwave_SRCS})

#############################################################################

TARGET_LINK_LIBRARIES(kwave_core
    kwave
    kwavegui
    ${QT_AND_KDECORE_LIBS}
    kdeui
    kio
    artsc
    artsflow
)

ADD_DEPENDENCIES(kwave_core libkwavegui libkwave)

SET_TARGET_PROPERTIES(kwave_core PROPERTIES OUTPUT_NAME "kwave")

#############################################################################
### install files                                                         ###

# the main program
INSTALL(TARGETS kwave_core     DESTINATION ${DESTDIR}${KDE3_EXEDIR})

# other files
INSTALL(FILES kwave.desktop    DESTINATION ${KDE3_APPSDIR}/Multimedia)
INSTALL(FILES menus.config     DESTINATION ${KDE3_CONFDIR}/kwave)
INSTALL(FILES ${kwave_PIXMAPS} DESTINATION ${KDE3_DATADIR}/kwave/pics)
INSTALL(FILES x-aifc.desktop x-ircam.desktop DESTINATION ${KDE3_MIMEDIR}/audio)

# scaled application icons
FOREACH(_col locolor hicolor)
    FOREACH(_dim 16x16 22x22 32x32 48x48)
        INSTALL(
            FILES ${CMAKE_CURRENT_BINARY_DIR}/kwave-${_dim}.png
            DESTINATION ${KDE3_ICONDIR}/${_col}/${_dim}/apps/kwave.png
        )
    ENDFOREACH(_dim)
ENDFOREACH(_col)

#############################################################################
#############################################################################
