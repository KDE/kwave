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
# - plugins/dialogs/about/module.h
#
# NOTE: - this should be considered a quick hack, no error 
#         checking is performed !!!
#       - there must be no additional spaces at the VERSION line
#         between the word "VERSION" and the "=".
#
# 08.11.1999 THE, adapted the script to work for the kwave project
#
# 12.11.1999 Martin Wilz, added date for .lsm file and changed file 
#            permissions of configure script back to executable
#
# 26.12.1999 THE, split the former "make_release" script into four parts:
#            - set_version: feeds a new version number/date into various files
#            - increment_release: starts a new release, version number will
#                                 be incremented, patchlevel will be removed
#            - increment_patchlevel: increases the patchlevel and leaves the
#                                    version number untouched
#            - make_patch: creates a patch file from the previous project
#                          version found in an archive directory to the
#                          current version
#            the configure script and Makefiles will no longer be modified

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
	} else
	if (a[1] == "Entered-date") {
	    printf("Entered-date:\t%s\n", toupper(newdate))
	} else 
	    print $0
	}' > kwave.lsm.new
mv kwave.lsm /tmp/kwave.lsm.old
mv kwave.lsm.new kwave.lsm

#
# update plugins/dialogs/about/module.h
#
SHORT_DATE=`(LANG=en; date -d "$NEW_DATE" +"%b %d, %Y")`
cat plugins/about/AboutDialog.cpp | \
	awk -v newver=$NEW_VERSION -v newdate="$SHORT_DATE" '{ 
	split($0, a, " ") } {
	if ((a[1] == "#define") && (a[2] == "KWAVE_VERSION")) {
	    printf("#define KWAVE_VERSION \"%s\"\n", newver)
	} else 
	if ((a[1] == "#define") && (a[2] == "KWAVE_VERSION_DATE")) {
	    printf("#define KWAVE_VERSION_DATE \"%s\"\n", newdate)
	} else
	    print $0
	}' > AboutDialog.cpp.new
mv plugins/about/AboutDialog.cpp /tmp/AboutDialog.cpp.old
mv AboutDialog.cpp.new plugins/about/AboutDialog.cpp

echo "new version numbers set."

#
# end of file
#
