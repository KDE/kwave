#############################################################################
##    Kwave                - po/CMakeLists.txt
##                           -------------------
##    begin                : Sat May 12 2007
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
### build list of .po files                                               ###

FOREACH(_lang ${KWAVE_BUILD_LINGUAS})
    STRING(REGEX MATCH "^[^:]+" _lang_short ${_lang})
    IF (NOT ${_lang_short} STREQUAL "en")
        SET(_po_files ${_po_files} ${_lang_short}.po)
    ENDIF (NOT ${_lang_short} STREQUAL "en")
ENDFOREACH(_lang ${KWAVE_BUILD_LINGUAS})

GETTEXT_CREATE_TRANSLATIONS(kwave.pot ALL ${_po_files})

#############################################################################
#############################################################################
