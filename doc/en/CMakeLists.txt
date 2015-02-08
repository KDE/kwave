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
### "make doc"                                                            ###

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
    DESTINATION ${HTML_INSTALL_DIR}/en/kwave/
)

#############################################################################
### generate the handbook, KDE environment                                ###

kde4_create_handbook(index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en SUBDIR kwave)

#############################################################################
#############################################################################
