#############################################################################
##    Kwave                - cmake/KwaveL10N.cmake l10n support
##                           -------------------
##    begin                : Sat Sep 13 2008
##    copyright            : (C) 2008 by Thomas Eschenbacher
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

FIND_REQUIRED_PROGRAM(FIND_EXECUTABLE find)
FIND_REQUIRED_PROGRAM(MSGCAT_EXECUTABLE msgcat)
FIND_REQUIRED_PROGRAM(XGETTEXT_EXECUTABLE xgettext)
FIND_REQUIRED_PROGRAM(MSGMERGE_EXECUTABLE msgmerge)
FIND_REQUIRED_PROGRAM(MSGFMT_EXECUTABLE msgfmt)

SET(PO_SRC_DIR "${CMAKE_SOURCE_DIR}/po")
SET(PO_BIN_DIR "${CMAKE_BINARY_DIR}/po")

#############################################################################
### get environment variable LINGUAS, default to all languages            ###

SET(LINGUAS "$ENV{LINGUAS}")
STRING(REGEX REPLACE "[ \t]+" \; OUTPUT "${LINGUAS}")
SEPARATE_ARGUMENTS(LINGUAS)
IF ("${LINGUAS}" STREQUAL "")
    SET(LINGUAS "*")
ENDIF ("${LINGUAS}" STREQUAL "")

#############################################################################
### checks whether a language has been requested per configuration       ###

MACRO(CHECK_LANG _lang _result)
    # take only languages that have been requested
    SET(${_result} TRUE)
    IF (NOT "${LINGUAS}" STREQUAL "*")
        LIST(FIND LINGUAS "${${_lang}}" _found)
        IF (_found LESS 0)
            SET(${_result} FALSE)
        ENDIF (_found LESS 0)
    ENDIF (NOT "${LINGUAS}" STREQUAL "*")
ENDMACRO(CHECK_LANG _lang _result)

#############################################################################
### find out which po files exist                                        ###

FILE(GLOB _existing_po_files "${PO_SRC_DIR}/*.po")
FOREACH(_po_file ${_existing_po_files})
    GET_FILENAME_COMPONENT(_lang "${_po_file}" NAME_WE)

    CHECK_LANG(_lang _take_it)
    IF (_take_it)
        LIST(APPEND KWAVE_BUILD_LINGUAS "${_lang}")
        LIST(APPEND _po_files "${_po_file}")
        MESSAGE(STATUS "Enabled GUI translation for ${_lang}")

        # handle generation and installation of the message catalog (gmo)
        SET(_gmo_file ${PO_BIN_DIR}/${_lang}.gmo)

        ADD_CUSTOM_COMMAND(
            OUTPUT ${_gmo_file}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${PO_BIN_DIR}
            COMMAND ${MSGFMT_EXECUTABLE} -o ${_gmo_file} ${_po_file}
            DEPENDS ${_po_file}
        )

        INSTALL(
            FILES ${_gmo_file}
            DESTINATION ${KDE_INSTALL_LOCALEDIR}/${_lang}/LC_MESSAGES
            RENAME kwave.mo
        )
        SET(_gmo_files ${_gmo_files} ${_gmo_file})

    ENDIF (_take_it)
ENDFOREACH(_po_file ${_existing_po_files})

#############################################################################
### processing of GUI translations if found                               ###

IF (NOT "${KWAVE_BUILD_LINGUAS}" STREQUAL "")

    IF ("${LINGUAS}" STREQUAL "*")
        MESSAGE(STATUS "LINGUAS not set, building for all supported languages")
    ENDIF ("${LINGUAS}" STREQUAL "*")

    IF (_existing_po_files)

        # build target "package-messages"
        ADD_CUSTOM_TARGET(package-messages ALL DEPENDS ${_gmo_files} )

        SET_DIRECTORY_PROPERTIES(PROPERTIES
            ADDITIONAL_MAKE_CLEAN_FILES
            "${CMAKE_BINARY_DIR}/po"
        )
    ENDIF (_existing_po_files)

ELSE (NOT "${KWAVE_BUILD_LINGUAS}" STREQUAL "")

    MESSAGE(STATUS "Found no suitable language to build for")

ENDIF (NOT "${KWAVE_BUILD_LINGUAS}" STREQUAL "")

#############################################################################
### show the progress of translations                                     ###

ADD_CUSTOM_TARGET(msgstats
    COMMAND ${CMAKE_SOURCE_DIR}/bin/msgstats.pl ${CMAKE_SOURCE_DIR}
)

#############################################################################
#############################################################################
