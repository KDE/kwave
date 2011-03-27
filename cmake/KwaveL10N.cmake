#############################################################################
##    Kwave                - cmake/KwaveL10N.cmake l10n support
##                           -------------------
##    begin                : Sat Sep 13 2008
##    copyright            : (C) 2008 by Thomas Eschenbacher
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
### get environment variable LINGUAS, default to all languages            ###

SET(LINGUAS "$ENV{LINGUAS}")
STRING(REGEX REPLACE "[ \t]+" \; OUTPUT "${LINGUAS}")
SEPARATE_ARGUMENTS(LINGUAS)
IF ("${LINGUAS}" STREQUAL "")
    SET(LINGUAS ${KWAVE_LINGUAS})
    MESSAGE(STATUS "LINGUAS not set, building for all supported languages")
ENDIF ("${LINGUAS}" STREQUAL "")

#############################################################################
### filter out languages that do not (or no longer) have a corresponding  ###
### .po file under po/... - the Gentoo build system steals files from the ###
### unpacked source directory without giving a notice and therefore might ###
### break our build!                                                      ###

FOREACH (_lingua ${LINGUAS})
    STRING(REGEX MATCH "^[^:]+" _lang_full ${_lingua})
    STRING(REGEX MATCH "^[^_]+" _lang ${_lang_full})
    SET(_pofile "${CMAKE_SOURCE_DIR}/po/${_lang}.po")
    IF ("${_lang}" STREQUAL "en")
        LIST(APPEND EXISTING_LINGUAS "${_lang}")
    ELSE ("${_lang}" STREQUAL "en")
        IF (EXISTS "${_pofile}")
            LIST(APPEND EXISTING_LINGUAS "${_lang}")
        ENDIF (EXISTS "${_pofile}")
    ENDIF ("${_lang}" STREQUAL "en")
ENDFOREACH(_lingua)

#############################################################################
### filter out only languages that are supported by Kwave                 ###

FOREACH (_lingua ${EXISTING_LINGUAS})
    STRING(REGEX MATCH "^[^:]+" _lingua_short_full ${_lingua})
    STRING(REGEX MATCH "^[^_]+" _lingua_short ${_lingua_short_full})
    FOREACH(_known_lang ${KWAVE_LINGUAS})
        STRING(REGEX MATCH "^[^:]+" _lang ${_known_lang})
        STRING(REGEX MATCH "[^:]+$$" _lang_name ${_known_lang})
        IF ("${_lang}" STREQUAL ${_lingua_short})
            SET(_new_entry "${_lang}:${_lang_name};")
            LIST(FIND KWAVE_BUILD_LINGUAS "${_new_entry}" _found)
            IF (_found LESS 0)
                LIST(APPEND KWAVE_BUILD_LINGUAS ${_new_entry})
                MESSAGE(STATUS "Enabled language support for ${_lingua_short} (${_lang_name})")
            ENDIF (_found LESS 0)
        ENDIF ("${_lang}" STREQUAL ${_lingua_short})
    ENDFOREACH(_known_lang)
ENDFOREACH(_lingua)

#############################################################################
### show a message if no suitable language found                          ###

IF ("${KWAVE_BUILD_LINGUAS}" STREQUAL "")
    MESSAGE(STATUS "Found no suitable language to build for")
ENDIF ("${KWAVE_BUILD_LINGUAS}" STREQUAL "")

#############################################################################
#############################################################################
