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

SET(plugin_goto_LIB_SRCS
    GotoPluginBase.cpp
    GotoPlugin.cpp
    GotoDialog.cpp
)

SET(plugin_goto_LIB_UI
    GotoDlg.ui
)

KWAVE_PLUGIN(goto)

#############################################################################

SET(plugin_insert_at_LIB_SRCS
    GotoPluginBase.cpp
    GotoDialog.cpp
    InsertAtPlugin.cpp
)

SET(plugin_insert_at_LIB_UI
    GotoDlg.ui
)

KWAVE_PLUGIN(insert_at)

#############################################################################
#############################################################################
