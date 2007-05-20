#############################################################################
##    Kwave                - cmake/MCOP.cmake
##                           -------------------
##    begin                : Tue May 08 2007
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

# create the skeletion and header file for mcop stuff
# usage: KDE_ADD_MCOP_IDL_FILES(foo_SRCS ${mcop_headers})
MACRO(KDE3_ADD_MCOP_IDL_FILES _sources)
    FOREACH (_current_FILE ${ARGN})

        GET_FILENAME_COMPONENT(_tmp_FILE ${_current_FILE} ABSOLUTE)
        GET_FILENAME_COMPONENT(_basename ${_tmp_FILE} NAME_WE)

        SET(_cc  ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cc)
        SET(_h   ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
        SET(_idl ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.idl)

        IF (NOT _tmp_FILE STREQUAL "${_idl}")
            ADD_CUSTOM_COMMAND(OUTPUT ${_idl}
                COMMAND cp
                ARGS ${_tmp_FILE} ${_idl}
                DEPENDS ${_tmp_FILE}
            )
        ENDIF (NOT _tmp_FILE STREQUAL "${_idl}")

        ADD_CUSTOM_COMMAND(OUTPUT ${_cc} ${_h}
            COMMAND ${KDE3_MCOPIDL_EXECUTABLE}
            ARGS -I${ARTS_INCLUDE_DIR} ${_idl}
            DEPENDS ${_idl}
            COMMENT "Generating ${_basename}.cc and ${_basename}.h"
        )

        SET(${_sources} ${${_sources}} ${_cc} ${_h})

        SET_SOURCE_FILES_PROPERTIES(${_cc}  PROPERTIES GENERATED true)
        SET_SOURCE_FILES_PROPERTIES(${_h}   PROPERTIES GENERATED true)
        SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
            "${_cc} ${_h}")

    ENDFOREACH (_current_FILE)

ENDMACRO(KDE3_ADD_MCOP_IDL_FILES)

#############################################################################
#############################################################################
