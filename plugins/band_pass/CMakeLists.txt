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
    c_filter_stuff.c
)

SET(plugin_band_pass_LIB_UI
    BandPassDlg.ui
)

KDE3_ADD_MCOP_IDL_FILES(plugin_band_pass_LIB_SRCS
    synth_band_pass.idl
)

KWAVE_PLUGIN(band_pass)

#############################################################################
#############################################################################
