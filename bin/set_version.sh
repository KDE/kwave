#!/bin/sh
#
# set_version - script to set version numbers of a project
# 
# 20.02.1999 by Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de> (THE)
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
	-v newdate=`date --date=$NEW_DATE +%d%b%Y` '{ 
	split($0, a, ":") } {
	if (a[1] == "Version") {
	    printf("Version:\t%s\n", newver)
	} else if (a[1] == "Entered-date") {
	    printf("Entered-date:\t%s\n", toupper(newdate))
	} else 
	    print $0
	}' > kwave.lsm.new
mv kwave.lsm /tmp/kwave.lsm.old
mv kwave.lsm.new kwave.lsm

#
# update the file configure.in
#
cat configure.in | \
	awk -v newver=$NEW_VERSION '{ 
	split($0, a, "(") } {
	if (a[1] == "AC_INIT") {
	    printf("AC_INIT(kwave,%s)\n", newver)
	} else if (a[1] == "AM_INIT_AUTOMAKE") {
	    printf("AM_INIT_AUTOMAKE(kwave,%s)\n", newver)
	} else
	    print $0
	}' > configure.in.new
mv configure.in /tmp/configure.in.old
mv configure.in.new configure.in

echo "new version numbers set."

#
# end of file
#
