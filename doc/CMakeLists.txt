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

FIND_PACKAGE(RequiredProgram REQUIRED)

SET(_insert_changes ${PROJECT_SOURCE_DIR}/bin/insert-changes.sh)
SET(_help_en_docbook_orig ${CMAKE_CURRENT_SOURCE_DIR}/help_en.docbook)
SET(_help_en_docbook ${CMAKE_CURRENT_BINARY_DIR}/help_en-work.docbook)
SET(_help_en_shifted ${CMAKE_CURRENT_BINARY_DIR}/help_en-shifted.docbook)
SET(_help_en_pot ${CMAKE_CURRENT_BINARY_DIR}/help_en.pot)
SET(_changes_docbook ${CMAKE_CURRENT_BINARY_DIR}/changes.docbook)
SET(_changes ${PROJECT_SOURCE_DIR}/CHANGES)
SET(_version ${PROJECT_SOURCE_DIR}/VERSION)
SET(_changes_to_docbook ${PROJECT_SOURCE_DIR}/bin/changes-to-docbook.awk)

FIND_REQUIRED_PROGRAM(CONVERT_EXECUTABLE convert)
FIND_REQUIRED_PROGRAM(CP_EXECUTABLE cp)
FIND_REQUIRED_PROGRAM(RM_EXECUTABLE rm)
FIND_REQUIRED_PROGRAM(AWK_EXECUTABLE gawk awk)
FIND_REQUIRED_PROGRAM(CAT_EXECUTABLE cat)
FIND_REQUIRED_PROGRAM(SED_EXECUTABLE sed)
FIND_REQUIRED_PROGRAM(RECODE_EXECUTABLE recode)
FIND_REQUIRED_PROGRAM(XML2POT_EXECUTABLE xml2pot)
FIND_REQUIRED_PROGRAM(MSGMERGE_EXECUTABLE msgmerge)
FIND_REQUIRED_PROGRAM(PO2XML_EXECUTABLE po2xml)
FIND_REQUIRED_PROGRAM(CHECKXML_EXECUTABLE checkXML)

#############################################################################
### update of the original docbook file                                   ###
ADD_CUSTOM_COMMAND(OUTPUT ${_help_en_docbook}
    COMMAND ${CAT_EXECUTABLE} ${_changes} |
        ${SED_EXECUTABLE} s/\\>\\>\\>//g |
        ${SED_EXECUTABLE} s/\\<\\<\\<//g |
        ${AWK_EXECUTABLE} -f ${_changes_to_docbook} |
        ${RECODE_EXECUTABLE} lat1..utf-8 > ${_changes_docbook}
    COMMAND ${CAT_EXECUTABLE} ${_help_en_docbook_orig} |
        ${_insert_changes} ${_changes_docbook} |
        ${AWK_EXECUTABLE}
        -v newdate=${KWAVE_VERSION_DATE_ISO}
        -v newrelease=${KWAVE_VERSION_FULL}
        -f ${PROJECT_SOURCE_DIR}/bin/date_release.awk
        > ${_help_en_docbook}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${_help_en_docbook} ${_help_en_docbook_orig}
    DEPENDS ${_changes}
    DEPENDS ${_version}
    DEPENDS ${_help_en_docbook_orig}
    COMMENT "Updating help_en.docbook"
)
SET_SOURCE_FILES_PROPERTIES(${_help_en_docbook} PROPERTIES GENERATED false)

#############################################################################
### help_en.docbook -> help_en.pot                                        ###

ADD_CUSTOM_COMMAND(OUTPUT ${_help_en_pot}
    COMMAND ${XML2POT_EXECUTABLE} ${_help_en_docbook} > ${_help_en_pot}
    DEPENDS ${_help_en_docbook}
)

#############################################################################
### help_en.docbook -> help_en-shifted.docbook                            ###

ADD_CUSTOM_COMMAND(OUTPUT ${_help_en_shifted}
    COMMAND ${CAT_EXECUTABLE} ${_help_en_docbook} |
            ${SED_EXECUTABLE} s/^[[:blank:]]*// >
            ${_help_en_shifted}
    DEPENDS ${_help_en_docbook}
)

#############################################################################
### target for cleaning generated HTML help                               ###

ADD_CUSTOM_TARGET(html_clean
    COMMAND ${RM_EXECUTABLE} -R -f ${CMAKE_CURRENT_BINARY_DIR}/html
)

#############################################################################
### macro for generating the help for each language                       ###

MACRO(KWAVE_TRANSLATE_DOC _sources _html_sources)

    FOREACH (_lang_and_name ${ARGN})
        STRING(REGEX MATCH "^[^:]+" _lang ${_lang_and_name})
        STRING(REGEX MATCH "[^:]+$$" _lang_name ${_lang_and_name})

        SET(_po ${CMAKE_CURRENT_BINARY_DIR}/help_${_lang}-tmp.po)
        SET(_po_orig ${CMAKE_CURRENT_SOURCE_DIR}/help_${_lang}.po)
        SET(_html_dir ${CMAKE_CURRENT_BINARY_DIR}/html/${_lang})
        SET(_html ${_html_dir}/index.html)
        SET(_common_dir ${KDE4_HTML_INSTALL_DIR}/${_lang}/common)

        # translation of the po and the docbook file not for english
        IF (NOT ${_lang} STREQUAL "en")
            SET(_docbook ${CMAKE_CURRENT_BINARY_DIR}/help_${_lang}.docbook)
            GET_FILENAME_COMPONENT(_docbook_basename ${_docbook} NAME)

            # translated po file
            ADD_CUSTOM_COMMAND(OUTPUT ${_po}
                COMMAND ${MSGMERGE_EXECUTABLE}
                    -o ${_po} ${_po_orig} ${_help_en_pot}
                COMMAND ${CMAKE_COMMAND} -E copy ${_po} ${_po_orig}
                DEPENDS ${_help_en_pot}
            )

            # translated docbook file
            ADD_CUSTOM_COMMAND(OUTPUT ${_docbook}
                COMMAND ${PO2XML_EXECUTABLE} ${_help_en_shifted} ${_po} |
                    ${SED_EXECUTABLE}
                    s/^\\<\\!ENTITY\\ %\\ English\\ /\\<\\!ENTITY\\ %\\ ${_lang_name}\\ /g
                    > ${_docbook}
                COMMAND ${CHECKXML_EXECUTABLE} ${_docbook}
                DEPENDS ${_po}
                DEPENDS ${_help_en_shifted}
                COMMENT STATUS "Building ${_docbook_basename} (${_lang_name})"
            )
        ELSE (NOT ${_lang} STREQUAL "en")
            SET(_docbook ${_help_en_docbook})
            GET_FILENAME_COMPONENT(_docbook_basename ${_docbook} NAME)
        ENDIF (NOT ${_lang} STREQUAL "en")

        # generated search index index_<lang>.cache.bz2
        SET(_index "${CMAKE_CURRENT_BINARY_DIR}/index_${_lang}.cache.bz2")
        ADD_CUSTOM_COMMAND(OUTPUT ${_index}
            COMMAND ${KDE4_MEINPROC_EXECUTABLE}
                    --check --cache ${_index} ${_docbook}
            DEPENDS ${_docbook}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

        # special handling for the GPL license file
        SET(_gpl_translated "${KDE4_HTML_INSTALL_DIR}/${_lang}/common/gpl-translated.html")
        SET(_gpl_english    "${KDE4_HTML_INSTALL_DIR}/en/common/gpl-license.html")
        IF (${_lang} STREQUAL "en")
            SET(_gpl ${_gpl_english})
        ELSE (${_lang} STREQUAL "en")
            IF (EXISTS ${_gpl_translated})
                SET(_gpl ${_gpl_translated})
            ELSE (EXISTS ${_gpl_translated})
                SET(_gpl ${_gpl_english})
            ENDIF (EXISTS ${_gpl_translated})
        ENDIF (${_lang} STREQUAL "en")

        ADD_CUSTOM_COMMAND(OUTPUT ${_html_dir}/gpl-translated.html
            COMMAND mkdir -p ${_html_dir}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${_gpl} ${_html_dir}/gpl-translated.html
        )
        SET(${_html_sources} ${${_html_sources}} ${_html_dir}/gpl-translated.html)

        # generated HTML help
        ADD_CUSTOM_COMMAND(OUTPUT ${_html}
            COMMAND mkdir -p ${_html_dir}
            COMMAND cd ${_html_dir} &&
                    ${KDE4_MEINPROC_EXECUTABLE} --check ${_docbook}
            COMMAND ${RM_EXECUTABLE} -Rf ${_html_dir}/common
            COMMAND mkdir -p ${_html_dir}/common
            COMMAND ${CONVERT_EXECUTABLE}
                ${CMAKE_SOURCE_DIR}/kwave/toolbar/zoomall.xpm
                ${_html_dir}/toolbar_zoomall.png
            COMMAND ${CONVERT_EXECUTABLE}
                ${CMAKE_SOURCE_DIR}/kwave/toolbar/zoomnormal.xpm
                ${_html_dir}/toolbar_zoomnormal.png
            COMMAND ${CONVERT_EXECUTABLE}
                ${CMAKE_SOURCE_DIR}/kwave/toolbar/zoomin.xpm
                ${_html_dir}/toolbar_zoomin.png
            COMMAND ${CONVERT_EXECUTABLE}
                ${CMAKE_SOURCE_DIR}/kwave/toolbar/zoomout.xpm
                ${_html_dir}/toolbar_zoomout.png
            COMMAND ${CONVERT_EXECUTABLE}
                ${CMAKE_SOURCE_DIR}/kwave/toolbar/zoomrange.xpm
                ${_html_dir}/toolbar_zoorange.png
            COMMAND ${CP_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/${_lang}/*.png ${_html_dir}
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${KDE4_HTML_INSTALL_DIR}/en/common ${_html_dir}/common/
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${_common_dir} ${_html_dir}/common/
            COMMAND cd ${_html_dir} && ${CMAKE_CURRENT_SOURCE_DIR}/fix-common
            DEPENDS ${_docbook}
            DEPENDS ${_html_dir}/gpl-translated.html
            COMMENT STATUS "Generating HTML help for ${_lang} (${_lang_name})"
        )

        SET(${_html_sources} ${${_html_sources}} ${_html})
        SET(${_sources} ${${_sources}} ${_docbook} ${_index})

        # installation of the docbook file
        INSTALL(
            FILES ${_docbook}
            DESTINATION ${KDE4_HTML_INSTALL_DIR}/${_lang}/kwave
            RENAME index.docbook
        )
        SET(_docbook_all ${_docbook_all} ${_docbook})

        # installation of the index.cache.bz2 file
        INSTALL(
            FILES ${_index}
            DESTINATION ${KDE4_HTML_INSTALL_DIR}/${_lang}/kwave
            RENAME index.cache.bz2
        )
        SET(_index_all ${_index_all} ${_index})

        # installation of the *.png files
        FILE(GLOB _png_files_${_lang} "${CMAKE_CURRENT_SOURCE_DIR}/${_lang}/*.png")
        INSTALL(FILES ${_png_files_${_lang}}
            DESTINATION ${KDE4_HTML_INSTALL_DIR}/${_lang}/kwave/
        )

        # install and uninstall of the symlinks to the "common" dirs
        INSTALL(CODE "
            MESSAGE(STATUS \"Installing \$ENV{DESTDIR}${KDE4_HTML_INSTALL_DIR}/${_lang}/kwave/common\")
        ")
        INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND}
            -E create_symlink
            \"../../en/common\"
            \"\$ENV{DESTDIR}${KDE4_HTML_INSTALL_DIR}/${_lang}/kwave/common\"
        )")

        # add the symlink to the extra uninstall files
        SET(ENV{EXTRA_UNINSTALL_FILES} "$ENV{EXTRA_UNINSTALL_FILES}\;${KDE4_HTML_INSTALL_DIR}/${_lang}/kwave/common")

    ENDFOREACH(_lang_and_name)

ENDMACRO(KWAVE_TRANSLATE_DOC)

#############################################################################

KWAVE_TRANSLATE_DOC(_docs _html_files ${KWAVE_BUILD_LINGUAS})

#############################################################################
### "make html_doc"                                                       ###

ADD_CUSTOM_TARGET(html_doc
    DEPENDS ${_docs}
    DEPENDS ${_html_files}
)

#############################################################################
### "make doc"                                                            ###

ADD_CUSTOM_TARGET(doc ALL
    DEPENDS ${_docbook_all} ${_index_all}
)

#############################################################################
#############################################################################
