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
SET(_help_en_docbook ${CMAKE_CURRENT_BINARY_DIR}/help_en.docbook)
SET(_help_en_shifted ${CMAKE_CURRENT_BINARY_DIR}/help_en-shifted.docbook)
SET(_help_en_pot ${CMAKE_CURRENT_BINARY_DIR}/help_en.pot)
SET(_changes_docbook ${CMAKE_CURRENT_BINARY_DIR}/changes.docbook)
SET(_changes ${PROJECT_SOURCE_DIR}/CHANGES)
SET(_version ${PROJECT_SOURCE_DIR}/VERSION)
SET(_changes_to_docbook ${PROJECT_SOURCE_DIR}/bin/changes-to-docbook.awk)

SET_SOURCE_FILES_PROPERTIES(${_changes_docbook} PROPERTIES GENERATED true)
SET_SOURCE_FILES_PROPERTIES(${_help_en_docbook} PROPERTIES GENERATED true)
SET_SOURCE_FILES_PROPERTIES(${_help_en_shifted} PROPERTIES GENERATED true)
SET_SOURCE_FILES_PROPERTIES(${_help_en_pot}     PROPERTIES GENERATED true)
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "
    ${_help_en_docbook}
    ${_help_en_shifted}
    ${_changes_docbook}
    ${_help_en_pot}
")

FIND_REQUIRED_PROGRAM(RM_EXECUTABLE rm)
FIND_REQUIRED_PROGRAM(AWK_EXECUTABLE gawk awk mawk)
FIND_REQUIRED_PROGRAM(CAT_EXECUTABLE cat)
FIND_REQUIRED_PROGRAM(SED_EXECUTABLE sed)
FIND_REQUIRED_PROGRAM(RECODE_EXECUTABLE recode)
FIND_REQUIRED_PROGRAM(XML2POT_EXECUTABLE xml2pot)
FIND_REQUIRED_PROGRAM(MSGMERGE_EXECUTABLE msgmerge)
FIND_REQUIRED_PROGRAM(PO2XML_EXECUTABLE po2xml)
FIND_REQUIRED_PROGRAM(CHECKXML_EXECUTABLE checkXML)
FIND_REQUIRED_PROGRAM(MEINPROC_EXECUTABLE meinproc)

#############################################################################
### CHANGES -> changes.docbook                                            ###

ADD_CUSTOM_COMMAND(OUTPUT ${_changes_docbook}
    COMMAND ${CAT_EXECUTABLE} ${_changes} |
        ${SED_EXECUTABLE} s/\\>\\>\\>//g |
        ${SED_EXECUTABLE} s/\\<\\<\\<//g |
        ${AWK_EXECUTABLE} -f ${_changes_to_docbook} |
        ${RECODE_EXECUTABLE} lat1..utf-8 > ${_changes_docbook}
    DEPENDS ${_changes} ${_changes_to_docbook}
)

#############################################################################
### update help_en.docbook                                                ###

ADD_CUSTOM_COMMAND(OUTPUT ${_help_en_docbook}
    COMMAND ${CAT_EXECUTABLE} ${_help_en_docbook_orig} |
        ${_insert_changes} changes.docbook |
        ${AWK_EXECUTABLE}
        -v newdate=${KWAVE_VERSION_DATE_ISO}
        -v newrelease=${KWAVE_VERSION_FULL}
        -f ${PROJECT_SOURCE_DIR}/bin/date_release.awk
        > ${_help_en_docbook}
    COMMAND cp ${_help_en_docbook} ${_help_en_docbook_orig}
    DEPENDS ${_changes_docbook}
    DEPENDS ${_help_en_docbook_orig}
    DEPENDS ${_insert_changes}
    DEPENDS ${_changes}
    DEPENDS ${_version}
    COMMENT "Updating help_en.docbook"
)

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
)

#############################################################################
### macro for generating the help for each language                       ###

MACRO(KWAVE_TRANSLATE_DOC _sources)

    FOREACH (_lang_and_name ${ARGN})
        STRING(REGEX MATCH "^[^:]+" _lang ${_lang_and_name})
        STRING(REGEX MATCH "[^:]+$$" _lang_name ${_lang_and_name})

        SET(_po ${CMAKE_CURRENT_BINARY_DIR}/help_${_lang}.po)
        SET(_po_orig ${CMAKE_CURRENT_SOURCE_DIR}/help_${_lang}.po)
        SET(_docbook ${CMAKE_CURRENT_BINARY_DIR}/help_${_lang}.docbook)
        GET_FILENAME_COMPONENT(_docbook_basename ${_docbook} NAME)
        SET(_html_dir ${CMAKE_CURRENT_BINARY_DIR}/${_lang})
        SET(_html ${_html_dir}/index.html)
        SET(_common_dir ${KDE3_HTMLDIR}/${_lang}/common)

        # translation of the po and the docbook file not for english
        IF (NOT ${_lang} STREQUAL "en")
            SET(_pot ${_help_en_pot})

            # translated po file
            ADD_CUSTOM_COMMAND(OUTPUT ${_po}
                COMMAND ${MSGMERGE_EXECUTABLE}
                    -o ${_po} ${_po_orig} ${_pot}
                COMMAND ${CAT_EXECUTABLE} ${_po} > ${_po_orig}
                DEPENDS ${_pot} ${_po_orig}
            )

            # translated docbook file
            ADD_CUSTOM_COMMAND(OUTPUT ${_docbook}
                COMMAND ${PO2XML_EXECUTABLE} ${_help_en_shifted} ${_po} |
                    ${SED_EXECUTABLE}
                    s/^\\<\\!ENTITY\\ %\\ English\\ /\\<\\!ENTITY\\ %\\ ${_lang_name}\\ /g
                    > ${_docbook}
                COMMAND ${CHECKXML_EXECUTABLE} ${_docbook}
                DEPENDS ${_help_en_shifted}
                DEPENDS ${_po}
                COMMENT STATUS "Building ${_docbook_basename} (${_lang_name})"
            )
        ENDIF (NOT ${_lang} STREQUAL "en")

        # special handling for the GPL license file
        IF (${_lang} STREQUAL "en")
            SET(_gpl ${KDE3_HTMLDIR}/${_lang}/common/gpl-license.html)
        ELSE (${_lang} STREQUAL "en")
            SET(_gpl ${KDE3_HTMLDIR}/${_lang}/docs/common/gpl-translated.html)
        ENDIF (${_lang} STREQUAL "en")
        ADD_CUSTOM_COMMAND(OUTPUT ${_html_dir}/gpl-translated.html
            COMMAND mkdir -p ${_html_dir}
            COMMAND cp ${_gpl} ${_html_dir}/gpl-translated.html
        )
        SET(${_sources} ${${_sources}} ${_html_dir}/gpl-translated.html)

        # generated HTML help
        ADD_CUSTOM_COMMAND(OUTPUT ${_html}
            COMMAND mkdir -p ${_html_dir}
            COMMAND cd ${_html_dir} &&
                    ${MEINPROC_EXECUTABLE} --check ${_docbook}
            COMMAND ${RM_EXECUTABLE} -Rf ${_html_dir}/common
            COMMAND mkdir -p ${_html_dir}/common
            COMMAND cp -R ${CMAKE_CURRENT_SOURCE_DIR}/${_lang}/*.png
                          ${_html_dir}
            COMMAND cp -R ${_common_dir}/* ${_html_dir}/common/
            COMMAND cd ${_html_dir} && ${CMAKE_CURRENT_SOURCE_DIR}/fix-common
            DEPENDS ${_docbook}
            DEPENDS ${_html_dir}/gpl-translated.html
            COMMENT STATUS "Generating HTML help for ${_lang} (${_lang_name})"
        )

        # cleanup of generated HTML help
        ADD_CUSTOM_TARGET(html_clean_${_lang}
            COMMAND ${RM_EXECUTABLE} -f ${CMAKE_CURRENT_BINARY_DIR}/${_lang}/*.html
            COMMAND ${RM_EXECUTABLE} -Rf ${CMAKE_CURRENT_BINARY_DIR}/${_lang}/common
            COMMENT STATUS "Cleaning HTML help for ${_lang} (${_lang_name})"
        )
        ADD_DEPENDENCIES(html_clean html_clean_${_lang})

        # handling of generated sources / cleanups
        IF (NOT ${_lang} STREQUAL "en")
            SET_SOURCE_FILES_PROPERTIES(${_po}      PROPERTIES GENERATED true)
            SET_SOURCE_FILES_PROPERTIES(${_docbook} PROPERTIES GENERATED true)
        ENDIF (NOT ${_lang} STREQUAL "en")
        SET_SOURCE_FILES_PROPERTIES(${_html_dir} PROPERTIES GENERATED true)

        SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
            "${_po} ${_docbook} ${_html_dir}")

        SET(${_sources} ${${_sources}} ${_docbook} ${_html})

    ENDFOREACH(_lang_and_name)

ENDMACRO(KWAVE_TRANSLATE_DOC)

#############################################################################

KWAVE_TRANSLATE_DOC(_docs ${KWAVE_LINGUAS})

ADD_CUSTOM_TARGET(html_doc
    DEPENDS ${_docs}
)

#############################################################################

#original Makefile.am contents follow:
#
# DOCDIR = $(DESTDIR)${kde_htmldir}

#install-data-local:
#	echo installing docs in $(DOCDIR)
#	for lang in ${LANGUAGES}; do \
#	{ \
#	    $(mkinstalldirs) ${DOCDIR}/$${lang}/$(PACKAGE); \
#	    $(INSTALL_DATA) help_$${lang}.docbook ${DOCDIR}/$${lang}/$(PACKAGE)/index.docbook; \
#	    $(INSTALL_DATA) $${lang}/*.png ${DOCDIR}/$${lang}/$(PACKAGE) ; \
#	    ( cd ${DOCDIR}/$${lang}/${PACKAGE} ; \
#	    for file in `ls ../../en/${PACKAGE}/*.png`; do \
#	    { \
#		if test ! -e `basename $${file}` ; then ln -sf $${file} . ; fi; \
#	    }; done ); \
#	    if test ! -e ${DOCDIR}/$${lang}/$(PACKAGE)/common ; then \
#	    rm -f ${DOCDIR}/$${lang}/$(PACKAGE)/common ; fi ; \
#	    ( cd ${DOCDIR}/$${lang}/${PACKAGE} ; ln -sf ../common . ) ; \
#	}; done
#	cd ${DOCDIR} ; \
#	if test ! -e default/$(PACKAGE) -a -e default ; then ln -sf ../en/$(PACKAGE) default; fi;
#
#uninstall-local:
#	for lang in ${LANGUAGES}; do { \
#	    rm -R -f ${DOCDIR}/$${lang}/${PACKAGE} ; \
#	    rmdir -p --ignore-fail-on-non-empty $(DOCDIR)/$$lang >/dev/null 2>&1; \
#	    if test -L ${DOCDIR}/default/$(PACKAGE); then rm -f ${DOCDIR}/default/$(PACKAGE); fi; \
#	}; done

#############################################################################
#############################################################################
