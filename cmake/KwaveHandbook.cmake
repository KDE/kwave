#############################################################################
##    Kwave                - cmake/KwaveHandbook.cmake
##                           -------------------
##    begin                : Wed Feb 18 2015
##    copyright            : (C) 2015 by Thomas Eschenbacher
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

# auto detect this language (to make this file re-usable)
GET_FILENAME_COMPONENT(_lang ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

SET(_common_dir ${HTML_INSTALL_DIR}/${_lang}/common)
SET(_html_dir ${CMAKE_BINARY_DIR}/doc/html/${_lang})

#############################################################################
### png files with the toolbar icons                                      ###

FILE(GLOB _toolbar_icons "${CMAKE_SOURCE_DIR}/kwave/toolbar/*.svgz")
FOREACH(_toolbar_icon ${_toolbar_icons})
    GET_FILENAME_COMPONENT(_svgz_file ${_toolbar_icon} NAME)
    STRING(REPLACE ".svgz" ".png" _png_file ${_svgz_file})
    SET(_toolbar_png ${CMAKE_CURRENT_BINARY_DIR}/toolbar_${_png_file})
    SVG2PNG(${_toolbar_icon} ${_toolbar_png} ${_png_file})
    SET(_toolbar_pngs "${_toolbar_pngs}" "${_toolbar_png}")
ENDFOREACH(_toolbar_icon ${_toolbar_icons})

#############################################################################
### "make html_doc"                                                       ###

FILE(GLOB _docbook_files "${CMAKE_CURRENT_SOURCE_DIR}/*.docbook")
FILE(GLOB _png_files "${CMAKE_SOURCE_DIR}/doc/${_lang}/*.png")

ADD_CUSTOM_TARGET(html_doc_${_lang}
    COMMENT "Generating HTML documentation for ${_lang}"
    DEPENDS ${_toolbar_pngs}
    DEPENDS ${_docbook_files}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${_html_dir}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_html_dir}
    COMMAND cd ${_html_dir} && ${KDE4_MEINPROC_EXECUTABLE}
            --check ${CMAKE_CURRENT_SOURCE_DIR}/index.docbook
    COMMAND ${CMAKE_COMMAND} -E make_directory   ${_html_dir}/common
    COMMAND test -z \"${_png_files}\" || ${CP_EXECUTABLE} ${_png_files} ${_html_dir}
    COMMAND ${CP_EXECUTABLE} ${_toolbar_pngs} ${_html_dir}
    COMMAND ${CP_EXECUTABLE} ${_common_dir}/* ${_html_dir}/common/
    COMMAND ${CP_EXECUTABLE} -n ${CMAKE_SOURCE_DIR}/doc/en/*.png ${_html_dir}
    COMMAND ${CP_EXECUTABLE} -n ${HTML_INSTALL_DIR}/en/common/* ${_html_dir}/common/
    COMMAND cd ${_html_dir} && ${CMAKE_SOURCE_DIR}/doc/fix-common
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

ADD_DEPENDENCIES(html_doc html_doc_${_lang})

#############################################################################
### generate and install the icons                                        ###

ADD_CUSTOM_TARGET(generate_icons_${_lang}
    ALL
    DEPENDS ${_toolbar_pngs}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
INSTALL(FILES
    ${_toolbar_pngs}
    DESTINATION ${HTML_INSTALL_DIR}/${_lang}/kwave/
)

#############################################################################
### generate the handbook, KDE environment                                ###

KDE4_CREATE_HANDBOOK(
    index.docbook
    INSTALL_DESTINATION ${HTML_INSTALL_DIR}/${_lang}
    SUBDIR kwave
)

#############################################################################
#############################################################################
