#############################################################################
##    Kwave                - cmake/FindRequiredProgram.cmake
##                           -------------------
##    begin                : Mon May 14 2007
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

# like FIND_PROGRAM, but show status message or abort if nothing found
# usage: FIND_REQUIRED_PROGRAM(variable name1 [name2] ...)
MACRO(FIND_REQUIRED_PROGRAM _variable)

    FIND_PROGRAM(${_variable} NAMES ${ARGN})

    IF (${_variable})
        GET_FILENAME_COMPONENT(_basename ${${_variable}} NAME_WE)
        MESSAGE(STATUS "Found ${_basename}: ${${_variable}}")
    ELSE (${_variable})
        MESSAGE(FATAL_ERROR "Unable to find executable for ${ARGN}")
    ENDIF (${_variable})

ENDMACRO(FIND_REQUIRED_PROGRAM)

#############################################################################
#############################################################################
