#############################################################################
##    Kwave                - plugins/lowpass/CMakeLists.txt
##                           -------------------
##    begin                : Sat Jun 02 2007
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

SET(plugin_lowpass_LIB_SRCS
    LowPassDialog.cpp
    LowPassFilter.cpp
    LowPassPlugin.cpp
)

SET(plugin_lowpass_LIB_UI
    LowPassDlg.ui
)

KWAVE_PLUGIN(lowpass)

#############################################################################
#############################################################################
