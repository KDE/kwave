#!/bin/sh
############################################################################
#   set_version.sh - script to set version numbers of a project
#                            -------------------
#   begin                : Sat Feb 20 1999
#   copyright            : (C) 1999 by Thomas Eschenbacher
#   email                : Thomas.Eschenbacher@gmx.de
############################################################################
#
############################################################################
#                                                                          #
#    This program is free software; you can redistribute it and/or modify  #
#    it under the terms of the GNU General Public License as published by  #
#    the Free Software Foundation; either version 2 of the License, or     #
#    (at your option) any later version.                                   #
#                                                                          #
############################################################################
#
# parameters:
# $1 = project root directory
# $2 = new version number
# $3 = new version's date+time
#
# All version numbers consist of major.minor.revision (e.g. 0.4.2) Only the
# last number, the release will be incremented. Any suffix (patchlevel)
# will be removed.
#
# the updated files are:
# - VERSION
# - kwave.lsm
# - plugins/about/AboutDialog.cpp
#
# NOTE: - this should be considered a quick hack, no error
#         checking is performed !!!
#       - there must be no additional spaces at the VERSION line
#         between the word "VERSION" and the "=".
#
# 1999-11-08 THE, adapted the script to work for the kwave project
#
# 1999-11-12 Martin Wilz, added date for .lsm file and changed file
#            permissions of configure script back to executable
#
# 1999-12-26 THE, split the former "make_release" script into four parts:
#            - set_version: feeds a new version number/date into various files
#            - increment_release: starts a new release, version number will
#                                 be incremented, patchlevel will be removed
#            - increment_patchlevel: increases the patchlevel and leaves the
#                                    version number untouched
#            - make_patch: creates a patch file from the previous project
#                          version found in an archive directory to the
#                          current version
#            the configure script and Makefiles will no longer be modified
#
# 2002-02-20 THE, no longer modifying about plugin
#
# 2003-06-21 THE, also setting AM_INIT_AUTOMAKE in configure.in
#
# 2007-07-22 THE, removed modification of configure.in due to change to cmake
#

# uncomment the next line for debugging
# set -x

cd $1
NEW_VERSION=$2
NEW_DATE="$3"

#
# show the new settings to the user
#
echo "   new version        : "\"$NEW_VERSION\"
echo "   new version's date : "\"$NEW_DATE\"

#
# update the VERSION file (by simply creating a new one)
#
echo $NEW_VERSION > VERSION

#
# update the file kwave.lsm
#
cat kwave.lsm | awk -v newver=$NEW_VERSION \
	-v newdate=`LC_ALL=en_EN date --date=$NEW_DATE +%d%b%Y` '{
	split($0, a, ":") } {
	if (a[1] == "Version") {
	    printf("Version:\t%s\n", newver)
	} else if (a[1] == "Entered-date") {
	    printf("Entered-date:\t%s\n", toupper(newdate))
	} else
	    print $0
	}' > kwave.lsm.new && \
mv kwave.lsm /tmp/kwave.lsm.old && \
mv kwave.lsm.new kwave.lsm

#
# update the docbook file
#
NEW_TAG=`echo ${NEW_VERSION} | sed s/\\\./_/g`
cat doc/help_en.docbook | \
    sed s/\<\!ENTITY\ version\ \"*.*.*\"\>/\<\!ENTITY\ version\ \"${NEW_VERSION}\"\>/g | \
    sed s/\<\!ENTITY\ version_tag\ \"*.*.*\"\>/\<\!ENTITY\ version_tag\ \"${NEW_TAG}\"\>/g | \
    sed s/\<date\>....-..-..\<\\/date\>/\<date\>${NEW_DATE}\<\\/date\>/g \
    > doc/help_en.docbook.new && \
mv doc/help_en.docbook doc/help_en.docbook.old && \
mv doc/help_en.docbook.new doc/help_en.docbook

echo "new version numbers set."

#
# end of file
#
