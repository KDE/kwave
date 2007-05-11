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

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE3_INCLUDE_DIR}
    ${QT_INCLUDE_DIR}
)

#############################################################################

SET(libkwavemt_STAT_SRCS
    AsyncSync.cpp
    SharedLock.cpp
    SharedLockGuard.cpp
    Thread.cpp
    ThreadsafeX11Guard.cpp
)

#############################################################################

KDE3_AUTOMOC(${libkwavemt_STAT_SRCS})
ADD_LIBRARY(libkwavemt STATIC ${libkwavemt_STAT_SRCS})
SET_TARGET_PROPERTIES(libkwavemt PROPERTIES OUTPUT_NAME "kwavemt")

#############################################################################
#############################################################################
