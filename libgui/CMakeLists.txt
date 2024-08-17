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

    Colors.h
    CurveWidget.h
    FileDialog.h
    FilterPlugin.h
    FrequencyResponseWidget.h
    HMSTimeWidget.h
    ImageView.h
    InvertableSpinBox.h
    LabelItem.h
    LabelPropertiesWidget.h
    MenuNode.h
    MenuGroup.h
    MenuItem.h
    MenuList.h
    MenuSub.h
    MenuRoot.h
    MenuManager.h
    MouseMark.h
    MultiStateWidget.h
    OverViewCache.h
    OverViewWidget.h
    ScaleWidget.h
    SelectionBorderItem.h
    SelectionItem.h
    SelectionTracker.h
    SelectTimeWidget.h
    SignalView.h
    SignalWidget.h
    TrackPixmap.h
    TrackView.h
    TreeWidgetWrapper.h
    ViewItem.h
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

TARGET_LINK_LIBRARIES(libkwavegui
    libkwave
    Qt::Core
    Qt::Concurrent
    Qt::Widgets
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::IconThemes
    KF6::KIOFileWidgets
    KF6::WidgetsAddons
    KF6::XmlGui
)

SET_TARGET_PROPERTIES(libkwavegui PROPERTIES OUTPUT_NAME "kwavegui")
SET_TARGET_PROPERTIES(libkwavegui PROPERTIES LINK_FLAGS
    "-Wl,--export-dynamic")
SET_TARGET_PROPERTIES(libkwavegui PROPERTIES
    VERSION ${KWAVE_VERSION} SOVERSION ${KWAVE_VERSION_MAJOR} )
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

ECM_GENERATE_EXPORT_HEADER(libkwavegui
    VERSION ${KWAVE_VERSION}
)

#############################################################################
### install files                                                         ###

INSTALL(TARGETS libkwavegui LIBRARY DESTINATION ${KDE_INSTALL_LIBDIR} NAMELINK_SKIP)

#############################################################################
#############################################################################
