#############################################################################
##    Kwave                - cmake/CheckIncludeFilesCXX.cmake
##                           -------------------
##    begin                : Tue Oct 13 2015
##    copyright            : (C) 2015 by Thomas Eschenbacher
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
