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
    Colors.cpp
    CurveWidget.cpp
    FileDialog.cpp
    FilterPlugin.cpp
    FrequencyResponseWidget.cpp
    HMSTimeWidget.cpp
    ImageView.cpp
    InvertableSpinBox.cpp
    LabelItem.cpp
    LabelPropertiesWidget.cpp
    MenuNode.cpp
    MenuGroup.cpp
    MenuItem.cpp
    MenuList.cpp
    MenuSub.cpp
    MenuRoot.cpp
    MenuManager.cpp
    MouseMark.cpp
    MultiStateWidget.cpp
    OverViewCache.cpp
    OverViewWidget.cpp
    ScaleWidget.cpp
    SelectionTracker.cpp
    SelectTimeWidget.cpp
    SignalView.cpp
    SignalWidget.cpp
    TrackPixmap.cpp
    TrackView.cpp
    TreeWidgetWrapper.cpp
    ViewItem.cpp
)

#############################################################################

SET(libkwavegui_LIB_UI
    HMSTimeWidgetBase.ui
    LabelPropertiesWidgetBase.ui
    SelectTimeWidgetBase.ui
)

#############################################################################

KI18N_WRAP_UI(libkwavegui_LIB_SRCS ${libkwavegui_LIB_UI} )
ADD_LIBRARY(libkwavegui SHARED ${libkwavegui_LIB_SRCS})

ADD_DEPENDENCIES(libkwavegui libkwave)

TARGET_LINK_LIBRARIES(libkwavegui
    kwave
    Qt5::Core
    Qt5::Concurrent
    Qt5::Widgets
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::IconThemes
    KF5::KIOFileWidgets
    KF5::WidgetsAddons
    KF5::XmlGui
)

SET_TARGET_PROPERTIES(libkwavegui PROPERTIES OUTPUT_NAME "kwavegui")
SET_TARGET_PROPERTIES(libkwavegui PROPERTIES LINK_FLAGS
    "-Wl,--export-dynamic")
SET_TARGET_PROPERTIES(libkwavegui PROPERTIES
    VERSION ${KWAVE_SOVERSION} SOVERSION ${KWAVE_VERSION_MAJOR} )
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

#############################################################################
### install files                                                         ###

INSTALL(TARGETS libkwavegui LIBRARY DESTINATION ${LIB_INSTALL_DIR} NAMELINK_SKIP)

#############################################################################
#############################################################################
