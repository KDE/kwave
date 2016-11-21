#!/bin/bash
#############################################################################
##    svn-update-l10n.sh   - update/fetch translations from anonsvn.kde.org
##                           -------------------
##    begin                : Sun Feb 08 2015
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

set -e
# set -x

CWD=`dirname $0`
CMAKE_SOURCE_DIR=$1

# REMOTE="svn+ssh://svn@svn.kde.org/home/kde/trunk/l10n-kf5/"
REMOTE="svn://anonsvn.kde.org/home/kde/trunk/l10n-kf5/"

REPOSITORY="${CMAKE_SOURCE_DIR}/l10n-kf5"
PO_DIR="${CMAKE_SOURCE_DIR}/po"
DOC_DIR="${CMAKE_SOURCE_DIR}/doc"

CATEGORY="kdemultimedia"
PO_FILE="kwave.po"
DESKTOP_PO_FILE="desktop_${CATEGORY}_kwave.po"
LANG_NAMES_FILE="${DOC_DIR}/teamnames"

function checkout() {
    local before=$1
    local part=$2
    shift 2
    local rest="$@"

    result=1

    test ! -z ${before} && before="${before}/"
    part=${before}${part}

    if test ! -e "${part}"; then
	svn update --quiet --depth empty "${part}@" || true 2>/dev/null
	if test ! -e "${part}"; then
	    result=0
	    return;
	fi
    fi

    if test ! -z "${rest}"; then
	checkout "${part}" $@
    fi
}

# make sure the repository exists and check it out if not
if test ! -e "${REPOSITORY}/.svn"; then
    svn checkout --quiet --depth empty ${REMOTE} ${REPOSITORY}
fi

# change into the local svn repository
cd "${REPOSITORY}"

# clean up leftovers in case we were aborted last time
svn cleanup

# get the list of all relevant sub directories and language names
svn update --quiet subdirs teamnames scripts

# create a new/empty "po" dir if necessary
mkdir -p "${PO_DIR}"

# check out all missing directories and files (recursively)
if test -z ${LINGUAS} ; then
    LINGUAS=`cat subdirs | grep -v x-test`
fi
FOUND_LINGUAS=""
FOUND_POFILES=""
FOUND_HANDBOOKS=""
rm -f ${LANG_NAMES_FILE}
touch ${LANG_NAMES_FILE}

for lang in ${LINGUAS}; do

    lang_team=`cat teamnames | grep ^${lang}=`

    echo -n "processing ${lang} - `echo ${lang_team} | cut -d = -f 2`... "

    # get handbook and screenshots
    if test -e "${lang}/docmessages/${CATEGORY}/kwave.po" ; then
	FOUND_HANDBOOKS="${FOUND_HANDBOOKS} ${lang}"
	echo ${lang_team} >> ${LANG_NAMES_FILE}
	if test -e "${lang}/docmessages/language" ; then
	    checkout "" "${lang}" "docmessages" "language"
	fi
    else
	if test ! -e "${lang}/docs/${CATEGORY}" ; then
	    checkout "" "${lang}" "docs" "${CATEGORY}"
	fi
	if test -e "${lang}/docs/${CATEGORY}" ; then
	    svn update --quiet "${lang}/docs/${CATEGORY}/kwave"
	fi
	checkout "" "${lang}" "docmessages" "${CATEGORY}" "${PO_FILE}"
	if [ ${result} == 1 ] ; then
	    FOUND_HANDBOOKS="${FOUND_HANDBOOKS} ${lang}"
	    echo ${lang_team} >> ${LANG_NAMES_FILE}
	    if test -e "${lang}/docmessages/language" ; then
		checkout "" "${lang}" "docmessages" "language"
	    fi
	fi
    fi

    # get translation of desktop files
    if test ! -e "${lang}/messages/${CATEGORY}/${DESKTOP_PO_FILE}" ; then
	checkout "" "${lang}" "messages"    "${CATEGORY}" "${DESKTOP_PO_FILE}"
    fi

    # GUI translation is mandantory
    if test ! -e "${lang}/messages/${CATEGORY}/${PO_FILE}" ; then
	checkout "" "${lang}" "messages"    "${CATEGORY}" "${PO_FILE}"
    fi
    if test -e "${lang}/messages/${CATEGORY}/${PO_FILE}" ; then
	FOUND_LINGUAS="${FOUND_LINGUAS} ${lang}"
	cp "${lang}/messages/${CATEGORY}/${PO_FILE}" "${PO_DIR}/${lang}.po"
	FOUND_POFILES="${FOUND_POFILES} ${lang}.po"
    fi
    echo "done"
done

if test ! -e "templates/messages/${CATEGORY}/desktop_${CATEGORY}_kwave.pot" ; then
    checkout "" "templates" "messages" "${CATEGORY}" "desktop_${CATEGORY}_kwave.pot"
fi
if test ! -e "templates/messages/${CATEGORY}/kwave.appdata.pot" ; then
    checkout "" "templates" "messages" "${CATEGORY}" "kwave.appdata.pot"
fi
if test ! -e "templates/messages/${CATEGORY}/kwave.pot" ; then
    checkout "" "templates" "messages" "${CATEGORY}" "kwave.pot"
fi

# update all existing files in the repository
svn update

# remove po files that vanished from kde servers
cd ${PO_DIR}
for file in *.po ; do
    if ! [[ ${FOUND_POFILES} =~ ${file} ]] ; then
	rm -fv ${PO_DIR}/${file}
    fi
done

# generate all missing index.docbook files
# or update the existing ones if necessary
cd "${REPOSITORY}"
for lang in ${FOUND_HANDBOOKS}; do

    if test -e "${lang}/docs/${CATEGORY}/kwave" ; then
	svn update "${lang}/docs/${CATEGORY}/kwave"
    else
	mkdir -p "${lang}/docs/${CATEGORY}/kwave"
    fi


    if ! test -e "${lang}/docs/${CATEGORY}/kwave/index.docbook" ; then
	echo -n "${lang}: creating missing index.docbook... "
	scripts/update_xml ${lang} ${CATEGORY} kwave > /dev/null
	echo "done"
    else
	if test "${lang}/docs/${CATEGORY}/kwave/index.docbook" -ot \
	         "${lang}/docmessages/${CATEGORY}/kwave.po" ; then
	    echo -n "${lang}: index.docbook is out of date - updating... "
	    scripts/update_xml ${lang} ${CATEGORY} kwave > /dev/null
	    echo "done"
	fi
    fi
done

# install the files needed for the handbook
for lang in ${FOUND_HANDBOOKS}; do
    rm -Rf "${DOC_DIR}/${lang}"

    if test -e "${lang}/docs/${CATEGORY}/kwave/index.docbook" ; then
	mkdir -p "${DOC_DIR}/${lang}"
	cp "${lang}/docs/${CATEGORY}/kwave/index.docbook" "${DOC_DIR}/${lang}/"
	cp "${DOC_DIR}/en/CMakeLists.txt" \
	   "${DOC_DIR}/${lang}/CMakeLists.txt"

	checkout "" "${lang}" "docs"        "${CATEGORY}" "kwave"
	if [ ${result} == 1 ]; then
	    svn update --quiet --set-depth infinity ${lang}/docs/${CATEGORY}/kwave
	    mkdir -p "${DOC_DIR}/${lang}"
	    cp -u "${lang}/docs/${CATEGORY}/kwave"/* "${DOC_DIR}/${lang}"/
	fi
    fi
done

set +e
svn status

# show translation statistics
${CWD}/msgstats.pl ${CMAKE_SOURCE_DIR}

### EOF ###
