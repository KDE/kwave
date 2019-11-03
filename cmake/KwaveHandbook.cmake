#############################################################################
##    Kwave                - cmake/KwaveHandbook.cmake
##                           -------------------
##    begin                : Wed Feb 18 2015
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

# auto detect this language (to make this file re-usable)
GET_FILENAME_COMPONENT(_lang ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

# /usr/share/help/de/kdoctools5-common
SET(_common_dir ${CMAKE_INSTALL_PREFIX}/share/help/${_lang}/kdoctools5-common)
SET(_common_en_dir ${CMAKE_INSTALL_PREFIX}/share/help/en/kdoctools5-common)
SET(_html_dir ${CMAKE_BINARY_DIR}/doc/html/${_lang})

#############################################################################
### png files with the toolbar icons                                      ###

FILE(GLOB _toolbar_icons "${CMAKE_SOURCE_DIR}/kwave/toolbar/*.svgz")
FOREACH(_toolbar_icon ${_toolbar_icons})
    GET_FILENAME_COMPONENT(_svgz_file ${_toolbar_icon} NAME)
    STRING(REPLACE "sc-actions-" "" _svgz_file_base ${_svgz_file})
    STRING(REPLACE ".svgz" ".png" _png_file ${_svgz_file_base})
    SET(_toolbar_png ${CMAKE_CURRENT_BINARY_DIR}/toolbar_${_png_file})
    SVG2PNG(${_toolbar_icon} ${_toolbar_png} ${_png_file})
    SET(_toolbar_pngs "${_toolbar_pngs}" "${_toolbar_png}")
ENDFOREACH(_toolbar_icon ${_toolbar_icons})

#############################################################################
### "make html_doc"                                                       ###

FILE(GLOB _docbook_files "${CMAKE_CURRENT_SOURCE_DIR}/*.docbook")
FILE(GLOB _png_files "${CMAKE_SOURCE_DIR}/doc/${_lang}/*.png")
GET_TARGET_PROPERTY(MEINPROC_EXECUTABLE ${KDOCTOOLS_MEINPROC_EXECUTABLE} LOCATION)

ADD_CUSTOM_TARGET(html_doc_${_lang}
    COMMENT "Generating HTML documentation for ${_lang}"
    DEPENDS ${_toolbar_pngs}
    DEPENDS ${_docbook_files}
    # start with an empty output (_html_dir)
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${_html_dir}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_html_dir}
    # create the HTML pages from docbook
    COMMAND cd ${_html_dir} && ${MEINPROC_EXECUTABLE}
            --check ${CMAKE_CURRENT_SOURCE_DIR}/index.docbook
    # copy the screenshots and converted toolbar icons
    COMMAND test -z \"${_png_files}\" || ${CP_EXECUTABLE} ${_png_files} ${_html_dir}
    COMMAND ${CP_EXECUTABLE} ${_toolbar_pngs}                           ${_html_dir}
    # take missing images from the English source directory
    COMMAND ${CP_EXECUTABLE} -n ${CMAKE_SOURCE_DIR}/doc/en/*.png        ${_html_dir}
    # copy files for the "common" directory
    COMMAND ${CMAKE_COMMAND} -E make_directory      ${_html_dir}/common
    COMMAND ${CP_EXECUTABLE} ${_common_dir}/*       ${_html_dir}/common/
    COMMAND ${CP_EXECUTABLE} -n ${_common_en_dir}/* ${_html_dir}/common/
    # fix wrong paths in the HTML pages
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

KDOCTOOLS_CREATE_HANDBOOK(
    index.docbook
    INSTALL_DESTINATION ${HTML_INSTALL_DIR}/${_lang}
    SUBDIR kwave
)

#############################################################################
#############################################################################
