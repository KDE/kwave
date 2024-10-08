#############################################################################
##    Kwave                - plugins/goto/CMakeLists.txt
##                           -------------------
##    begin                : Sat Dec 06 2008
##    copyright            : (C) 2008 by Thomas Eschenbacher
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

SET(plugin_goto_common_LIB_SRCS
    GotoDialog.cpp
    GotoPluginBase.cpp

    GotoDialog.h
    GotoPluginBase.h
)

SET(plugin_goto_common_LIB_UI
    GotoDlg.ui
)

KI18N_WRAP_UI(plugin_goto_common_LIB_SRCS ${plugin_goto_common_LIB_UI})
ADD_LIBRARY(plugin_goto_common STATIC ${plugin_goto_common_LIB_SRCS})
SET_TARGET_PROPERTIES(plugin_goto_common PROPERTIES ENABLE_EXPORTS ON)
set_target_properties(plugin_goto_common PROPERTIES POSITION_INDEPENDENT_CODE ON)
TARGET_LINK_LIBRARIES(plugin_goto_common
    libkwavegui
    libkwave
    Qt::Core
    Qt::Widgets
    KF6::ConfigWidgets
    KF6::CoreAddons
    KF6::I18n
)

#############################################################################

SET(plugin_goto_LIB_SRCS
    GotoPlugin.cpp
    GotoPlugin.h
)

SET(plugin_goto_LIBS plugin_goto_common)
KWAVE_PLUGIN(goto)

#############################################################################

SET(plugin_insert_at_LIB_SRCS
    InsertAtPlugin.cpp
    InsertAtPlugin.h
)

SET(plugin_insert_at_LIBS plugin_goto_common)
KWAVE_PLUGIN(insert_at)

#############################################################################
#############################################################################
