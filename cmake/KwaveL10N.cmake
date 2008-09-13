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

SET(LINGUAS $ENV{LINGUAS})
SEPARATE_ARGUMENTS(LINGUAS)
IF ("x${LINGUAS}" STREQUAL "x")
    SET(LINGUAS ${KWAVE_LINGUAS})
    MESSAGE(STATUS "LINGUAS not set, building for all supported languages")
ENDIF ("x${LINGUAS}" STREQUAL "x")

#############################################################################
### filter out only languages that are supported by Kwave                 ###

FOREACH (_lingua ${LINGUAS})
    STRING(REGEX MATCH "^[^:]+" _lingua_short ${_lingua})
    FOREACH(_known_lang ${KWAVE_LINGUAS})
        STRING(REGEX MATCH "^[^:]+" _lang ${_known_lang})
        STRING(REGEX MATCH "[^:]+$$" _lang_name ${_known_lang})
        IF (${_lang} STREQUAL ${_lingua_short})
            SET(KWAVE_BUILD_LINGUAS
                ${KWAVE_BUILD_LINGUAS}
                "${_lang}:${_lang_name};"
            )
            MESSAGE(STATUS "Enabled language support for ${_lingua_short} (${_lang_name})")
        ENDIF (${_lang} STREQUAL ${_lingua_short})
    ENDFOREACH(_known_lang)
ENDFOREACH(_lingua)

#############################################################################
### default to english if no suitable language found                      ###

IF ("x${KWAVE_BUILD_LINGUAS}" STREQUAL "x")
    MESSAGE(STATUS "Found no suitable language to build for, defaulting to english")
    SET(KWAVE_BUILD_LINGUAS en:English)
ENDIF ("x${KWAVE_BUILD_LINGUAS}" STREQUAL "x")

#############################################################################
#############################################################################
