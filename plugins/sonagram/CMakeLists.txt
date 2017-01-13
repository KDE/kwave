#############################################################################
##    Kwave                - plugins/sonagram/CMakeLists.txt
##                           -------------------
##    begin                : Sat Jun 02 2007
##    copyright            : (C) 2007 by Thomas Eschenbacher
##    email                : Thomas.Eschenbacher@gmx.de
#############################################################################
#
#############################################################################
#                                                                           #
# Redistribution and use in source and binary forms, with or without        #
# modification, are permitted provided that the following conditions        #
# are met:                                                                  #
#                                                                           #
# 1. Redistributions of source code must retain the above copyright         #
#    notice, this list of conditions and the following disclaimer.          #
# 2. Redistributions in binary form must reproduce the above copyright      #
#    notice, this list of conditions and the following disclaimer in the    #
#    documentation and/or other materials provided with the distribution.   #
#                                                                           #
# For details see the accompanying cmake/COPYING-CMAKE-SCRIPTS file.        #
#                                                                           #
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
