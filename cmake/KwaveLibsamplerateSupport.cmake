#############################################################################
##    Kwave                - cmake/KwaveLibsamplerateSupport.txt
##                           -------------------
##    begin                : Sat Jul 04 2009
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

INCLUDE(FindPkgConfig)
INCLUDE(UsePkgConfig)

#############################################################################
### check for libsamplerate headers and library                           ###

OPTION(WITH_SAMPLERATE "enable support for libsamplerate [default=on]" ON)
IF (WITH_SAMPLERATE)

    PKG_CHECK_MODULES(SAMPLERATE REQUIRED samplerate>=0.1.3)
    IF (NOT SAMPLERATE_FOUND)
        MESSAGE(FATAL_ERROR "libsamplerate not found")
    ENDIF(NOT SAMPLERATE_FOUND)

    SET(SAMPLERATE_LIBS samplerate)
    SET(HAVE_SAMPLERATE_SUPPORT ON)

    MESSAGE(STATUS "Found samplerate library in ${SAMPLERATE_LIBDIR}")
    MESSAGE(STATUS "Found samplerate headers in ${SAMPLERATE_INCLUDEDIR}")
    # MESSAGE(STATUS "    CFLAGS=${SAMPLERATE_CFLAGS}")
    # MESSAGE(STATUS "    LDLAGS=${SAMPLERATE_LDFLAGS}")
    # MESSAGE(STATUS "      LIBS=${SAMPLERATE_LIBS}")

ENDIF (WITH_SAMPLERATE)

#############################################################################
#############################################################################
