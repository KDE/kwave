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

CMAKE_SOURCE_DIR=$1

REMOTE="svn://anonsvn.kde.org/home/kde/trunk/l10n-kde4/"
REPOSITORY="${CMAKE_SOURCE_DIR}/l10n-kde4"
PO_DIR="${CMAKE_SOURCE_DIR}/po"
DOC_DIR="${CMAKE_SOURCE_DIR}/doc"

CATEGORY="kdereview"
PO_FILE="kwave.po"
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
	svn update --quiet --depth empty "${part}@"
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
LINGUAS=`cat subdirs | grep -v x-test`
FOUND_LINGUAS=""
FOUND_POFILES=""
FOUND_HANDBOOKS=""
rm -f ${LANG_NAMES_FILE}
touch ${LANG_NAMES_FILE}

for lang in ${LINGUAS}; do

    lang_team=`cat teamnames | grep ^${lang}=`

    echo -n "processing ${lang} - `echo ${lang_team} | cut -d = -f 2`... "

    # get handbook and screenshots
    checkout "" "${lang}" "docmessages" "${CATEGORY}" "${PO_FILE}"
    if [ ${result} == 1 ] ; then
	FOUND_HANDBOOKS="${FOUND_HANDBOOKS} ${lang}"
	mkdir -p "${DOC_DIR}/${lang}"
	cp -u "${lang}/docmessages/${CATEGORY}/kwave.po" "${DOC_DIR}/${lang}/"
	if test ! -e "${DOC_DIR}/${lang}/CMakeLists.txt"; then
	    cp -u "${DOC_DIR}/en/CMakeLists.txt" \
	          "${DOC_DIR}/${lang}/CMakeLists.txt"
	fi
	echo ${lang_team} >> ${LANG_NAMES_FILE}
    fi

    checkout "" "${lang}" "docs"        "${CATEGORY}" "kwave"
    if [ ${result} == 1 ]; then
	svn update --quiet --set-depth infinity ${lang}/docs/${CATEGORY}/kwave
	mkdir -p "${DOC_DIR}/${lang}"
	cp -u "${lang}/docs/${CATEGORY}/kwave"/* "${DOC_DIR}/${lang}"/
    fi

    # GUI translation is mandantory
    checkout "" "${lang}" "messages"    "${CATEGORY}" "${PO_FILE}"
    if [ ${result} == 1 ] ; then
	FOUND_LINGUAS="${FOUND_LINGUAS} ${lang}"
	cp "${lang}/messages/${CATEGORY}/${PO_FILE}" "${PO_DIR}/${lang}.po"
	FOUND_POFILES="${FOUND_POFILES} ${lang}.po"
    fi
    echo "done"
done

# update all existing files in the repository
svn update

# remove po files that vanished from kde servers
cd ${PO_DIR}
for file in *.po ; do
    if ! [[ ${FOUND_POFILES} =~ ${file} ]] ; then
	rm -fv ${PO_DIR}/${file}
    fi
done

# show translation statistics
cd "${REPOSITORY}"
cd ..
dir=`basename ${REPOSITORY}`
for catalog in `find "${dir}" -name "${PO_FILE}" | sort`; do
    echo -n ${catalog}": "
    msgfmt --statistics ${catalog}
done

### EOF ###
