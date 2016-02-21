#############################################################################
##    Kwave                - cmake/CheckIncludeFilesCXX.cmake
##                           -------------------
##    begin                : Tue Oct 13 2015
##    copyright            : (C) 2015 by Thomas Eschenbacher
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

INCLUDE(CheckIncludeFileCXX)

# like CHECK_INCLUDE_FILES, but for C++ header and aborts with a fatal error
# if one of them was not found
# usage: CHECK_INCLUDE_FILES_CXX(header1 [header2] ...)
MACRO(CHECK_INCLUDE_FILES_CXX INCLUDES)
    FOREACH(_include ${INCLUDES})
	CHECK_INCLUDE_FILE_CXX(${_include} HAVE_${_include})
	IF (NOT HAVE_${_include})
	    MESSAGE(FATAL_ERROR "unable to find the following C++ header file: ${_include}")
	ENDIF (NOT HAVE_${_include})
    ENDFOREACH(_include ${INCLUDES})
ENDMACRO(CHECK_INCLUDE_FILES_CXX)

#############################################################################
#############################################################################
