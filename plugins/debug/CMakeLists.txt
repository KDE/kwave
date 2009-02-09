#############################################################################
##    Kwave                - plugins/debug/CMakeLists.txt
##                           -------------------
##    begin                : Mon Feb 02 2009
##    copyright            : (C) 2009 by Thomas Eschenbacher
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

IF (DEBUG)
    SET(plugin_debug_LIB_SRCS
	DebugPlugin.cpp
    )

    KWAVE_PLUGIN(debug)
ENDIF (DEBUG)

#############################################################################
#############################################################################
