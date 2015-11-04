#############################################################################
##    Kwave                - doc/CMakeLists.txt
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

SET(UPDATE_COMMAND_XREF    ${CMAKE_SOURCE_DIR}/bin/update-command-xref.pl)
SET(UPDATE_FILEINFO_XREF   ${CMAKE_SOURCE_DIR}/bin/update-fileinfo-xref.pl)
SET(UPDATE_PLUGIN_XREF     ${CMAKE_SOURCE_DIR}/bin/update-plugin-xref.pl)
SET(_index_docbook         ${CMAKE_CURRENT_SOURCE_DIR}/en/index.docbook)
SET(_index_docbook_updated ${CMAKE_CURRENT_BINARY_DIR}/index-updated.docbook)

FIND_REQUIRED_PROGRAM(CP_EXECUTABLE cp)

#############################################################################
### how to convert from svg to png                                        ###

FIND_PROGRAM(RSVG_EXECUTABLE NAMES rsvg-convert rsvg)
IF (RSVG_EXECUTABLE)
    GET_FILENAME_COMPONENT(_basename ${RSVG_EXECUTABLE} NAME_WE)
    MESSAGE(STATUS "Found ${_basename}: ${RSVG_EXECUTABLE}")
ENDIF (RSVG_EXECUTABLE)

SET(_test_icon "${CMAKE_SOURCE_DIR}/kwave/toolbar/sc-actions-kwave_zoom_in.svgz")
GET_FILENAME_COMPONENT(_svgz_file ${_test_icon} NAME)
STRING(REPLACE ".svgz" ".png" _png_file ${_svgz_file})

SET(RSVG_AVAILABLE NO)
IF (RSVG_EXECUTABLE)
    # try to convert one icon, to see if "rsvg" really works
    EXECUTE_PROCESS(
	COMMAND ${RSVG_EXECUTABLE} -w 32 -h 32 -f png ${_test_icon} -o "${_png_file}"
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	RESULT_VARIABLE RSVG_CONVERT_RESULT
	OUTPUT_VARIABLE _stdout
	ERROR_VARIABLE _stderr
    )
    IF (${RSVG_CONVERT_RESULT} EQUAL 0)
	MESSAGE(STATUS "Converting svg to png with ${RSVG_EXECUTABLE}")
	MACRO(SVG2PNG _infile _outfile _name)
	    ADD_CUSTOM_COMMAND(OUTPUT ${_outfile}
		COMMAND ${RSVG_EXECUTABLE} -w 32 -h 32 -f png ${_infile} -o ${_outfile}
		DEPENDS ${_infile}
		COMMENT STATUS "Converting ${_name}"
	    )
	ENDMACRO(SVG2PNG)

	SET(RSVG_AVAILABLE YES)
    ELSE (${RSVG_CONVERT_RESULT} EQUAL 0)
	MESSAGE(STATUS "Found rsvg but conversion failed, falling back to convert from ImageMagick")
    ENDIF (${RSVG_CONVERT_RESULT} EQUAL 0)
ENDIF (RSVG_EXECUTABLE)

IF (NOT RSVG_AVAILABLE)
    SET(CONVERT_AVAILABLE NO)
    FIND_PROGRAM(CONVERT_EXECUTABLE NAMES convert)
    IF (CONVERT_EXECUTABLE)
	GET_FILENAME_COMPONENT(_basename ${CONVERT_EXECUTABLE} NAME_WE)
	MESSAGE(STATUS "Found ${_basename}: ${CONVERT_EXECUTABLE}")

	# try to convert one icon, to see if "convert" has proper svg support
	EXECUTE_PROCESS(
	    COMMAND ${CONVERT_EXECUTABLE} -resize 32x32 ${_test_icon} "${_png_file}"
	    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	    RESULT_VARIABLE SVG_CONVERT_RESULT
	    OUTPUT_VARIABLE _stdout
	    ERROR_VARIABLE _stderr
	)
	IF (${SVG_CONVERT_RESULT} EQUAL 0)
	    MESSAGE(STATUS "Converting svg to png with ${CONVERT_EXECUTABLE}")
	    MACRO(SVG2PNG _infile _outfile _name)
		ADD_CUSTOM_COMMAND(OUTPUT ${_outfile}
		    COMMAND ${CONVERT_EXECUTABLE} -resize 32x32 ${_infile} ${_outfile}
		    DEPENDS ${_infile}
		    COMMENT STATUS "Converting ${_name}"
		)
	    ENDMACRO(SVG2PNG)
	    SET(CONVERT_AVAILABLE YES)
	ENDIF (${SVG_CONVERT_RESULT} EQUAL 0)
    ENDIF (CONVERT_EXECUTABLE)

    IF (NOT CONVERT_AVAILABLE)
        MESSAGE(FATAL_ERROR "
        your system lacks support for converting icons in svg format to png.

        You have three options:
        1) install the tool \"rsvg\"
        2) install a version of \"convert\" (ImageMagick) that has proper svg support
        3) omit building the online documentation by using the cmake option \"-DWITH_DOC=OFF\"
        ")
    ENDIF (NOT CONVERT_AVAILABLE)
ENDIF (NOT RSVG_AVAILABLE)

# clean up the temporary png file after conversion test
FILE(REMOVE "${CMAKE_BINARY_DIR}/${_png_file}")

#############################################################################
### "make update-handbook"                                                ###

ADD_CUSTOM_TARGET(update-handbook
    COMMAND ${UPDATE_COMMAND_XREF}
	${CMAKE_SOURCE_DIR}
	${_index_docbook}
	${_index_docbook_updated}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_index_docbook_updated} ${_index_docbook}

    COMMAND ${UPDATE_FILEINFO_XREF}
	${_index_docbook}
	${CMAKE_SOURCE_DIR}/libkwave/FileInfo.cpp
	${_index_docbook_updated}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_index_docbook_updated} ${_index_docbook}

    COMMAND ${UPDATE_PLUGIN_XREF}
	${CMAKE_SOURCE_DIR}/plugins
	${_index_docbook}
	${_index_docbook_updated}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_index_docbook_updated} ${_index_docbook}
)

#############################################################################
### handle the various languages                                          ###

# collects all "html_doc_${_lang}" sub targets
ADD_CUSTOM_TARGET(html_doc)

FILE(GLOB _files "${CMAKE_SOURCE_DIR}/doc/*/index.docbook")
FOREACH(_file ${_files})
    GET_FILENAME_COMPONENT(_subdir ${_file} DIRECTORY)
    GET_FILENAME_COMPONENT(_lang ${_subdir} NAME)

    # take only languages that have been requested
    SET(_take_it TRUE)
    IF (NOT "${LINGUAS}" STREQUAL "*")
	LIST(FIND LINGUAS "${_lang}" _found)
	IF (_found LESS 0)
	    SET(_take_it FALSE)
	ENDIF (_found LESS 0)
    ENDIF (NOT "${LINGUAS}" STREQUAL "*")

    IF (_take_it)
	MESSAGE(STATUS "Enabled handbook for ${_lang}")
	ADD_SUBDIRECTORY(${_lang})
    ENDIF (_take_it)

ENDFOREACH(_file ${_files})

#############################################################################
#############################################################################
