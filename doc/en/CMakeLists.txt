#############################################################################
##    Kwave                - doc/en/CMakeLists.txt
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

# auto detect this language (to make this file re-usable)
GET_FILENAME_COMPONENT(_lang ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

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
ADD_CUSTOM_TARGET(html_doc
    DEPENDS ${_toolbar_pngs}
    DEPENDS ${_docbook_files}
    COMMAND ${KDE4_MEINPROC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/index.docbook
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

#############################################################################
### generate and install the icons                                        ###

ADD_CUSTOM_TARGET(generate_icons
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
