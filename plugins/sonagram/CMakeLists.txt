#############################################################################
##    Kwave                - plugins/sonagram/CMakeLists.txt
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

INCLUDE(FindPkgConfig)
INCLUDE(UsePkgConfig)

#############################################################################
### check for FFTW v3 headers and library                                 ###

PKG_CHECK_MODULES(FFTW REQUIRED fftw3>=3.0)
IF (NOT FFTW_FOUND)
    MESSAGE(FATAL_ERROR "FFTW library not found")
ENDIF(NOT FFTW_FOUND)

PKG_CHECK_MODULES(FFTW fftw3)

MESSAGE(STATUS "Found FFTW library in ${FFTW_LIBDIR}")
MESSAGE(STATUS "Found FFTW headers in ${FFTW_INCLUDEDIR}")
# MESSAGE(STATUS "    CFLAGS=${FFTW_CFLAGS}")
# MESSAGE(STATUS "    LDLAGS=${FFTW_LDFLAGS}")

#############################################################################
### sonagram plugin                                                       ###

SET(plugin_sonagram_LIB_SRCS
    SonagramDialog.cpp
    SonagramPlugin.cpp
    SonagramWindow.cpp
)

SET(plugin_sonagram_LIB_UI
    SonagramDlg.ui
)

SET(plugin_sonagram_LIBS
    fftw3 m
)

KWAVE_PLUGIN(sonagram)

#############################################################################
#############################################################################
