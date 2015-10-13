#############################################################################
##    Kwave                - plugins/goto/CMakeLists.txt
##                           -------------------
##    begin                : Sat Dec 06 2008
##    copyright            : (C) 2008 by Thomas Eschenbacher
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

SET(plugin_goto_common_LIB_SRCS
    GotoDialog.cpp
    GotoPluginBase.cpp
)

SET(plugin_goto_common_LIB_UI
    GotoDlg.ui
)

KDE4_ADD_UI_FILES(plugin_goto_common_LIB_SRCS ${plugin_goto_common_LIB_UI})
QT4_AUTOMOC(${plugin_goto_common_LIB_SRCS})
KDE4_ADD_LIBRARY(plugin_goto_common STATIC ${plugin_goto_common_LIB_SRCS})
SET_TARGET_PROPERTIES(plugin_goto_common PROPERTIES ENABLE_EXPORTS ON)
ADD_DEPENDENCIES(plugin_goto_common libkwave libkwavegui)
TARGET_LINK_LIBRARIES(plugin_goto_common kwavegui kwave)

#############################################################################

SET(plugin_goto_LIB_SRCS
    GotoPlugin.cpp
)

SET(plugin_goto_LIBS plugin_goto_common)
KWAVE_PLUGIN(goto)

#############################################################################

SET(plugin_insert_at_LIB_SRCS
    InsertAtPlugin.cpp
)

SET(plugin_insert_at_LIBS plugin_goto_common)
KWAVE_PLUGIN(insert_at)

#############################################################################
#############################################################################
