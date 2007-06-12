#############################################################################
##    Kwave                - libgui/CMakeLists.txt
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

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE3_INCLUDE_DIR}
    ${QT_INCLUDE_DIR}
)

#############################################################################

SET(libkwavegui_LIB_SRCS
    CurveWidget.cpp
    ConfirmCancelProxy.cpp
    FileProgress.cpp
    FrequencyResponseWidget.cpp
    HMSTimeWidget.cpp
    IntValidatorProxy.cpp
    InvertableSpinBox.cpp
    KwaveFileDialog.cpp
    LabelPropertiesWidget.cpp
    MenuNode.cpp
    MenuGroup.cpp
    MenuItem.cpp
    MenuSub.cpp
    MenuRoot.cpp
    MenuManager.cpp
    MultiStateWidget.cpp
    OverViewCache.cpp
    OverViewWidget.cpp
    ScaleWidget.cpp
    SelectTimeWidget.cpp
    TrackPixmap.cpp
)

#############################################################################

SET(libkwavegui_LIB_UI
    HMSTimeWidgetBase.ui
    LabelPropertiesWidgetBase.ui
    SelectTimeWidgetBase.ui
)

#############################################################################

KDE3_ADD_UI_FILES(libkwavegui_LIB_SRCS ${libkwavegui_LIB_UI} )
KDE3_AUTOMOC(${libkwavegui_LIB_SRCS})
ADD_LIBRARY(libkwavegui SHARED ${libkwavegui_LIB_SRCS})
TARGET_LINK_LIBRARIES(libkwavegui  ${QT_AND_KDECORE_LIBS} )
SET_TARGET_PROPERTIES(libkwavegui PROPERTIES OUTPUT_NAME "kwavegui")
SET_TARGET_PROPERTIES(libkwavegui PROPERTIES LINK_FLAGS
    "-Wl,--export-dynamic")
SET_TARGET_PROPERTIES(libkwavegui PROPERTIES
    VERSION ${KWAVE_SOVERSION} SOVERSION ${KWAVE_VERSION_MAJOR} )

#############################################################################
### install files                                                         ###

INSTALL(TARGETS libkwavegui LIBRARY DESTINATION ${KDE3_LIB_DIR})

#############################################################################
#############################################################################
