#############################################################################
##    Kwave                - plugins/samplerate/CMakeLists.txt
##                           -------------------
##    begin                : Tue Jul 07 2009
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

IF (WITH_SAMPLERATE)

    SET(plugin_samplerate_LIB_SRCS
	RateConverter.cpp
	SampleRatePlugin.cpp
    )

    SET(plugin_samplerate_LIBS
	samplerate
    )

    KWAVE_PLUGIN(samplerate)

ENDIF (WITH_SAMPLERATE)

#############################################################################
#############################################################################
