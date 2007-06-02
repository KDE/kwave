#############################################################################
##    Kwave                - plugins/pitch_shift/CMakeLists.txt
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

SET(plugin_pitch_shift_LIB_SRCS
    PitchShiftDialog.cpp
    PitchShiftPlugin.cpp
)

SET(plugin_pitch_shift_LIB_UI
    PitchShiftDlg.ui
)

KDE3_ADD_MCOP_IDL_FILES(plugin_pitch_shift_LIB_SRCS
    synth_pitch_shift_bugfixed.idl
)

KWAVE_PLUGIN(pitch_shift)

#############################################################################
#############################################################################
