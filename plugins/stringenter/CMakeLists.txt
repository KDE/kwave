#############################################################################
##    Kwave                - plugins/stringenter/CMakeLists.txt
##                           -------------------
##    begin                : Sat Mar 14 2015
##    copyright            : (C) 2015 by Thomas Eschenbacher
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

SET(plugin_stringenter_LIB_SRCS
    StringEnterDialog.cpp
    StringEnterPlugin.cpp
)

SET(plugin_stringenter_LIB_UI
    StringEnterDlg.ui
)

KWAVE_PLUGIN(stringenter)

#############################################################################
#############################################################################
