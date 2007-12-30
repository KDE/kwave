#############################################################################
##    Kwave                - mt/CMakeLists.txt
##                           -------------------
##    begin                : Tue May 01 2007
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

#############################################################################

#############################################################################

SET(libkwavemt_STAT_SRCS
    Thread.cpp
)

#############################################################################

QT4_AUTOMOC(${libkwavemt_STAT_SRCS})
KDE4_ADD_LIBRARY(libkwavemt STATIC ${libkwavemt_STAT_SRCS})
SET_TARGET_PROPERTIES(libkwavemt PROPERTIES OUTPUT_NAME "kwavemt")
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

#############################################################################
#############################################################################
