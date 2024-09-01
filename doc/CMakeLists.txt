#############################################################################
##    Kwave                - doc/CMakeLists.txt
##                           -------------------
##    begin                : Sat May 12 2007
##    copyright            : (C) 2007 by Thomas Eschenbacher
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

SET(UPDATE_COMMAND_XREF    ${CMAKE_SOURCE_DIR}/bin/update-command-xref.pl)
SET(UPDATE_FILEINFO_XREF   ${CMAKE_SOURCE_DIR}/bin/update-fileinfo-xref.pl)
SET(UPDATE_PLUGIN_XREF     ${CMAKE_SOURCE_DIR}/bin/update-plugin-xref.pl)
SET(_index_docbook         ${CMAKE_CURRENT_SOURCE_DIR}/en/index.docbook)
SET(_index_docbook_updated ${CMAKE_CURRENT_BINARY_DIR}/index-updated.docbook)

FIND_REQUIRED_PROGRAM(CP_EXECUTABLE cp)

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
### "make html_doc_devel"                                                 ###

SET(_common_en_dir ${CMAKE_INSTALL_PREFIX}/share/help/en/kdoctools5-common)
SET(_html_dir_devel ${CMAKE_CURRENT_BINARY_DIR}/devel)
GET_TARGET_PROPERTY(MEINPROC_EXECUTABLE ${KDOCTOOLS_MEINPROC_EXECUTABLE} LOCATION)

ADD_CUSTOM_TARGET(html_doc_devel
    COMMENT "Generating HTML developer documentation"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/devel.docbook
    # start with an empty output (_html_dir)
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${_html_dir_devel}
    COMMAND ${CMAKE_COMMAND} -E make_directory   ${_html_dir_devel}
    # create the HTML pages from docbook
    COMMAND cd ${_html_dir_devel} && ${MEINPROC_EXECUTABLE}
            --check ${CMAKE_CURRENT_SOURCE_DIR}/devel.docbook
    # copy files for the "common" directory
    COMMAND ${CMAKE_COMMAND} -E make_directory      ${_html_dir_devel}/common
    COMMAND ${CP_EXECUTABLE} -n ${_common_en_dir}/* ${_html_dir_devel}/common/
    # fix wrong paths in the HTML pages
    COMMAND cd ${_html_dir_devel} && ${CMAKE_SOURCE_DIR}/doc/fix-common
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

#############################################################################
#############################################################################
