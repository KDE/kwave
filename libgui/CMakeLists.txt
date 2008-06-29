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

#############################################################################

SET(libkwavegui_LIB_SRCS
    CurveWidget.cpp
    ConfirmCancelProxy.cpp
    FrequencyResponseWidget.cpp
    HMSTimeWidget.cpp
    ImageView.cpp
    InvertableSpinBox.cpp
    KwaveFileDialog.cpp
    KwaveFilterPlugin.cpp
    LabelPropertiesWidget.cpp
    MenuNode.cpp
    MenuGroup.cpp
    MenuItem.cpp
    MenuSub.cpp
    MenuRoot.cpp
    MenuManager.cpp
    MouseMark.cpp
    MultiStateWidget.cpp
    OverViewCache.cpp
    OverViewWidget.cpp
    ScaleWidget.cpp
    SelectTimeWidget.cpp
    SignalWidget.cpp
    TrackPixmap.cpp
    TreeWidgetWrapper.cpp
    UndoModifyLabelAction.cpp
)

#############################################################################

SET(libkwavegui_LIB_UI
    HMSTimeWidgetBase.ui
    LabelPropertiesWidgetBase.ui
    SelectTimeWidgetBase.ui
)

#############################################################################

KDE4_ADD_UI_FILES(libkwavegui_LIB_SRCS ${libkwavegui_LIB_UI} )
QT4_AUTOMOC(${libkwavegui_LIB_SRCS})
KDE4_ADD_LIBRARY(libkwavegui SHARED ${libkwavegui_LIB_SRCS})

ADD_DEPENDENCIES(libkwavegui libkwave)

TARGET_LINK_LIBRARIES(libkwavegui
    kwave
    ${KDE4_KFILE_LIBS}
    ${QT_AND_KDECORE_LIBS}
)

SET_TARGET_PROPERTIES(libkwavegui PROPERTIES OUTPUT_NAME "kwavegui")
SET_TARGET_PROPERTIES(libkwavegui PROPERTIES LINK_FLAGS
    "-Wl,--export-dynamic")
SET_TARGET_PROPERTIES(libkwavegui PROPERTIES
    VERSION ${KWAVE_SOVERSION} SOVERSION ${KWAVE_VERSION_MAJOR} )
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

#############################################################################
### install files                                                         ###

INSTALL(TARGETS libkwavegui LIBRARY DESTINATION ${KDE4_LIB_INSTALL_DIR})

#############################################################################
#############################################################################
