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

INCLUDE(CheckLibraryExists)

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

IF (NOT HAVE_GSL)
    MESSAGE(FATAL_ERROR "
        could not find required version of GSL
        (GNU Scientific Library)
        please install the GSL and it's -devel package!
        see http://www.gnu.org/software/gsl
    ")
ENDIF (NOT HAVE_GSL)

SET(plugin_sonagram_LIB_SRCS
    ImageView.cpp
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

#############################################################################
#############################################################################
