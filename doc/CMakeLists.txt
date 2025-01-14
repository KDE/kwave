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

add_subdirectory(en)

SET(UPDATE_COMMAND_XREF    ${CMAKE_SOURCE_DIR}/bin/update-command-xref.pl)
SET(UPDATE_FILEINFO_XREF   ${CMAKE_SOURCE_DIR}/bin/update-fileinfo-xref.pl)
SET(UPDATE_PLUGIN_XREF     ${CMAKE_SOURCE_DIR}/bin/update-plugin-xref.pl)
SET(_index_docbook         ${CMAKE_CURRENT_SOURCE_DIR}/en/index.docbook)
SET(_index_docbook_updated ${CMAKE_CURRENT_BINARY_DIR}/index-updated.docbook)
GET_TARGET_PROPERTY(MEINPROC_EXECUTABLE ${KDOCTOOLS_MEINPROC_EXECUTABLE} LOCATION)

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
### html documentation for the various languages                          ###

SET(_stylesheet "${KDOCTOOLS_CUSTOMIZATION_DIR}/kde-web.xsl")

SET(_common_en_dir ${KDE_INSTALL_FULL_DOCBUNDLEDIR}/en/kdoctools6-common)
# some distros (Gentoo at least) customize where KDocTools gets installed
SET(_alternate_dir /usr/share/help/en/kdoctools6-common)
IF(NOT IS_DIRECTORY ${_common_en_dir} AND IS_DIRECTORY ${_alternate_dir})
    SET(_common_en_dir ${_alternate_dir})
ENDIF()

SET(_base_html_dir ${CMAKE_BINARY_DIR}/doc/html)
SET(_html_dir ${_base_html_dir}/en)

# collects all "html_doc_${_lang}" sub targets
ADD_CUSTOM_TARGET(html_doc
    COMMENT "Generating HTML documentation for en"
    DEPENDS ${CMAKE_SOURCE_DIR}/doc/en/index.docbook
    # start with an empty output (_html_dir)
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${_html_dir}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_html_dir}
    # create HTML pages from the docbook
    COMMAND cd ${_html_dir} && ${MEINPROC_EXECUTABLE}
            --check ${CMAKE_SOURCE_DIR}/doc/en/index.docbook --stylesheet ${_stylesheet}
    # copy the screenshots
    COMMAND ${CP_EXECUTABLE} -u ${CMAKE_SOURCE_DIR}/doc/en/*.png ${_html_dir}
    # copy the kdoctools common files where the generated html expects them
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_base_html_dir}/common
    COMMAND ${CP_EXECUTABLE} -u ${_common_en_dir}/* ${_base_html_dir}/common/
)

FILE(GLOB _files "${CMAKE_SOURCE_DIR}/po/*/docs/kwave/index.docbook")
FOREACH(_file ${_files})
    GET_FILENAME_COMPONENT(_subdir2 ${_file}    DIRECTORY)
    GET_FILENAME_COMPONENT(_subdir1 ${_subdir2} DIRECTORY)
    GET_FILENAME_COMPONENT(_subdir  ${_subdir1} DIRECTORY)
    GET_FILENAME_COMPONENT(_lang ${_subdir}     NAME)

    SET(_html_dir ${_base_html_dir}/${_lang})
    FILE(GLOB _png_files "${CMAKE_SOURCE_DIR}/po/${_lang}/docs/kwave/*.png")

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
        ADD_DEPENDENCIES(html_doc html_doc_${_lang})
        ADD_CUSTOM_TARGET(html_doc_${_lang}
            COMMENT "Generating HTML documentation for ${_lang}"
            DEPENDS ${_toolbar_pngs}

            # start with an empty output (_html_dir)
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${_html_dir}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${_html_dir}

            # create the HTML pages from docbook
            COMMAND cd ${_html_dir} && ${MEINPROC_EXECUTABLE} --check ${_file} --stylesheet ${_stylesheet}

            # copy the screenshots
            COMMAND test -z \"${_png_files}\" || ${CP_EXECUTABLE} ${_png_files} ${_html_dir}/

            # TODO: find a way to get all toolbar icons
            # COMMAND ${CP_EXECUTABLE} ${_toolbar_pngs}                           ${_html_dir}/

            # take missing images from the English source directory
            COMMAND ${CP_EXECUTABLE} -u ${CMAKE_SOURCE_DIR}/doc/en/*.png ${_html_dir}
        )
    ENDIF (_take_it)

ENDFOREACH(_file ${_files})

#############################################################################
### "make html_doc_devel"                                                 ###

SET(_html_dir_devel ${_base_html_dir}/devel)

ADD_CUSTOM_TARGET(html_doc_devel
    COMMENT "Generating HTML developer documentation"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/devel.docbook
    # start with an empty output (_html_dir)
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${_html_dir_devel}
    COMMAND ${CMAKE_COMMAND} -E make_directory   ${_html_dir_devel}
    # create the HTML pages from docbook
    COMMAND cd ${_html_dir_devel} && ${MEINPROC_EXECUTABLE}
            --check ${CMAKE_CURRENT_SOURCE_DIR}/devel.docbook --stylesheet ${_stylesheet}
    # copy the kdoctools common files where the generated html expects them
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_base_html_dir}/common
    COMMAND ${CP_EXECUTABLE} -u ${_common_en_dir}/* ${_base_html_dir}/common/
)

#############################################################################
#############################################################################
