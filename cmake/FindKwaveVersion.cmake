#############################################################################
##    Kwave                - cmake/FindKwaveVersion.cmake
##                           -------------------
##    begin                : Fri May 11 2007
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
### read the version info from the "VERSION" file in the toplevel dir     ###
FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/VERSION KWAVE_VERSION_RAW)

#############################################################################
### split into major.minor.release                                        ###
STRING(REGEX MATCHALL "([0-9]+)" KWAVE_VERSION_LIST ${KWAVE_VERSION_RAW})
LIST(GET KWAVE_VERSION_LIST 0 KWAVE_VERSION_MAJOR)
LIST(GET KWAVE_VERSION_LIST 1 KWAVE_VERSION_MINOR)
LIST(GET KWAVE_VERSION_LIST 2 KWAVE_VERSION_RELEASE)

#############################################################################
### re-assemble full version number: major.minor.release                  ###
SET(KWAVE_VERSION_FULL "${KWAVE_VERSION_MAJOR}.${KWAVE_VERSION_MINOR}.${KWAVE_VERSION_RELEASE}")

#############################################################################
### optional: a patchlevel separated with a "-"                           ###
LIST(LENGTH KWAVE_VERSION_LIST COUNT)
IF (${COUNT} GREATER 3)
    LIST(GET KWAVE_VERSION_LIST 3 KWAVE_VERSION_PATCHLEVEL)
    SET(KWAVE_VERSION_FULL "${KWAVE_VERSION_FULL}-${KWAVE_VERSION_PATCHLEVEL}")
ENDIF (${COUNT} GREATER 3)

#############################################################################
### version number for .so files                                          ###
SET(KWAVE_SOVERSION "${KWAVE_VERSION_MAJOR}.${KWAVE_VERSION_MINOR}.${KWAVE_VERSION_RELEASE}")

#############################################################################
### status output                                                         ###
MESSAGE(STATUS "Building Kwave version ${KWAVE_VERSION_FULL}")

#############################################################################
### ISO formated date                                                     ###

FIND_PACKAGE(RequiredProgram REQUIRED)
FIND_REQUIRED_PROGRAM(DATE_EXECUTABLE date)

EXECUTE_PROCESS(
    COMMAND
        ${DATE_EXECUTABLE} --iso
    OUTPUT_VARIABLE
        KWAVE_VERSION_DATE_ISO
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

#############################################################################
#############################################################################
