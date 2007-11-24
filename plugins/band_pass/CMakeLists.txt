#############################################################################
##    Kwave                - plugins/band_pass/CMakeLists.txt
##                           -------------------
##    begin                : Mon May 28 2007
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

SET(plugin_band_pass_LIB_SRCS
    BandPass.cpp
    BandPassDialog.cpp
    BandPassPlugin.cpp
)

SET(plugin_band_pass_LIB_UI
    BandPassDlg.ui
)

KWAVE_PLUGIN(band_pass)

#############################################################################
#############################################################################
