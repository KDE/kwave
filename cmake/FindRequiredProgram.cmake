#############################################################################
##    Kwave                - cmake/FindRequiredProgram.cmake
##                           -------------------
##    begin                : Mon May 14 2007
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
