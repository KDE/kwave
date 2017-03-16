#############################################################################
##    Kwave                - libgui/CMakeLists.txt
##                           -------------------
##    begin                : Tue May 01 2007
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
    SelectionBorderItem.cpp
    SelectionItem.cpp
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
    VERSION ${KWAVE_VERSION} SOVERSION ${KWAVE_VERSION_MAJOR} )
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

#############################################################################
### install files                                                         ###

INSTALL(TARGETS libkwavegui LIBRARY DESTINATION ${LIB_INSTALL_DIR} NAMELINK_SKIP)

#############################################################################
#############################################################################
