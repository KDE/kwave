#############################################################################
##    Kwave                - plugins/transcription/CMakeLists.txt
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

#INCLUDE(FindPkgConfig)
#INCLUDE(UsePkgConfig)

#############################################################################
### labeler plugin                                                        ###

SET(plugin_labeler_LIB_SRCS
    LabelerPlugin.cpp
    Labels.cpp
    gui/LabelView.cpp
)

#SET(plugin_trancription_LIB_UI
#    trancriptionDlg.ui
#)

KWAVE_PLUGIN(labeler)

#############################################################################
#############################################################################
