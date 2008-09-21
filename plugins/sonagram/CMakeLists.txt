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

OPTION(WITH_GSL "enable usage of libgsl [default=on]" ON)
IF (WITH_GSL)

INCLUDE(CheckLibraryExists)
INCLUDE(FindPkgConfig)

#############################################################################
### check for GSL headers and library                                     ###

CHECK_INCLUDE_FILES("gsl/gsl_complex.h;gsl/gsl_fft.h;gsl/gsl_fft_complex.h" HAVE_GSL_HEADERS)

SET(CMAKE_REQUIRED_LIBRARIES gsl gslcblas)
IF (HAVE_GSL_HEADERS)
    CHECK_LIBRARY_EXISTS(gsl gsl_fft_complex_wavetable_alloc "" HAVE_GSL_1)
    IF (HAVE_GSL_1)
        CHECK_LIBRARY_EXISTS(gsl gsl_fft_complex_wavetable_free "" HAVE_GSL_2)
    ENDIF (HAVE_GSL_1)
    IF (HAVE_GSL_2)
        CHECK_LIBRARY_EXISTS(gsl gsl_fft_complex_workspace_alloc "" HAVE_GSL_3)
    ENDIF (HAVE_GSL_2)
    IF (HAVE_GSL_3)
        CHECK_LIBRARY_EXISTS(gsl gsl_fft_complex_workspace_free "" HAVE_GSL_4)
    ENDIF (HAVE_GSL_3)
    IF (HAVE_GSL_3)
        CHECK_LIBRARY_EXISTS(gsl gsl_fft_complex_forward "" HAVE_GSL)
    ENDIF (HAVE_GSL_3)
ENDIF(HAVE_GSL_HEADERS)
SET(CMAKE_REQUIRED_LIBRARIES "")

#############################################################################
### check for GSL under GPL v2 / *NOT* GPL v3                             ###

PKG_CHECK_MODULES(GSL gsl)
IF (GSL_VERSION)
    PKG_CHECK_MODULES(GSL_GPLV2 REQUIRED gsl<=1.9)
    IF (NOT GSL_GPLV2_FOUND)
        MESSAGE(FATAL_ERROR "
            Found GSL version >= v1.10 which is under GPL-v3. Unfortunately
            this license is not compatible with the other liceses Kwave
            currently uses.

            Alternatively you can disable the usage of gsl with the cmake
            parameter -DWITH_GSL=OFF, which also disables the sonagram plugin.
        ")
    ENDIF(NOT GSL_GPLV2_FOUND)
ELSE (GSL_VERSION)
    SET(HAVE_GSL FALSE)
ENDIF(GSL_VERSION)

IF (NOT HAVE_GSL)
    MESSAGE(FATAL_ERROR "
        could not find required version of GSL
        (GNU Scientific Library)
        please install the GSL (best is v1.9) and it's -devel package!
        see http://www.gnu.org/software/gsl
    ")
ENDIF (NOT HAVE_GSL)

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
    gsl gslcblas
)

KWAVE_PLUGIN(sonagram)

ELSE(WITH_GSL)

    MESSAGE(STATUS "Skipping sonagram plugin as usage of libgsl is disabled")

ENDIF(WITH_GSL)

#############################################################################
#############################################################################
