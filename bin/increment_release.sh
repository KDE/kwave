#!/bin/sh
#
# increment_release - script to increment version numbers of a project
# 
# 20.02.1999 by Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de> (THE)
#
# This script parses the file "VERSION" in order to find a variable
# called "VERSION", splits it into version major, minor and release and
# updates some files with the new version number.
#
# All version numbers consist of major.minor.revision (e.g. 0.4.2) Only the 
# last number, the release will be incremented. Any suffix (patchlevel) 
# will be removed.
#
# the updated files are: 
# - VERSION    (the file itself)
# - configure
# - Makefile (if it exists)
# - kwave.lsm
# - plugins/dialogs/about/module.h
#
# NOTE: - this should be considered a quick hack, no error 
#         checking is performed !!!
#       - there must be no additional spaces at the VERSION line
#         between the word "VERSION" and the "=".
#
# 08.11.1999 THE, adapted the script to work for the kwave project
# 12.11.1999 Martin Wilz, added date for .lsm file and changed file permissions
# of configure script back to executable

# set -x

#
# get the current version number
#
VERSION=`cat VERSION`

echo -n "old version = $VERSION"

#
# get the actual version numbers
#

ACT_MAJOR=`echo $VERSION | \
		awk '{ split($0, a, ".") } END { printf "%03d", a[1] }'`
ACT_MINOR=`echo $VERSION |
		\awk '{ split($0, a, ".") } END { printf "%03d", a[2] }'`
ACT_REL=`echo $VERSION"." | \
		    awk '{ split($0,a,".") } END { print a[3] }'`
ACT_RELEASE=`echo $ACT_REL- | \
		    awk '{ split($0,a,"-") } END { printf "%03d", a[1] }'`
ACT_SUFFIX=`echo $ACT_REL- | \
		    awk '{ split($0,a,"-") } END { print a[2] }'`
if test "$ACT_SUFFIX" != ""; then
    export ACT_SUFFIX="-"$ACT_SUFFIX
fi

ACT_VERSION=$ACT_MAJOR"."$ACT_MINOR"."$ACT_RELEASE$ACT_SUFFIX
#
# generate the new version number
#
NEW_MAJOR=`echo $VERSION | \
		awk '{ split($0, a, ".") } END { print a[1] }'`
NEW_MINOR=`echo $VERSION | \
		awk '{ split($0, a, ".") } END { print a[2] }'`
NEW_RELEASE=`echo $VERSION | \
		awk '{ split($0, a, ".") } END { print a[3]+1 }'`
NEW_VERSION=$NEW_MAJOR"."$NEW_MINOR"."$NEW_RELEASE


# update the VERSION file (by simply creating a new one)
#
echo $NEW_VERSION > VERSION

#
# update the Makefile (an old copy is stored in /tmp)
#
if test -a Makefile ; then
    cat Makefile | awk -v newver=$NEW_VERSION '{ 
	split($0, a, "=") }  {
	if (a[1] == "VERSION ") {
	    printf("VERSION = %s\n", newver)
	} else 
	    print $0
	}' > /tmp/Makefile.new
    mv Makefile /tmp/Makefile.old
    mv /tmp/Makefile.new Makefile
fi

#
# update the configure script (an old copy is stored in /tmp)
#
cat configure | awk -v newver=$NEW_VERSION '{ 
	split($0, a, "=") } {
	if (a[1] == "VERSION") {
	    printf("VERSION=%s\n", newver)
	} else 
	    print $0
	}' > /tmp/configure.new
mv configure /tmp/configure.old
mv /tmp/configure.new configure
#change file permissions to executable
chmod 755 configure

#
# update the file kwave.lsm
#
DATE=`date`
cat kwave.lsm | awk -v newver=$NEW_VERSION -v newdate=$DATE '{ 
	split($0, a, ":") } {
	if (a[1] == "Version") {
	    printf("Version:\t%s\n", newver)
	} else
	if (a[1] == "Entered-date") {
	    printf("Entered-date:\t%s\n", newdate)
	} else 
	    print $0
	}' > /tmp/kwave.lsm.new
mv kwave.lsm /tmp/kwave.lsm.old
mv /tmp/kwave.lsm.new kwave.lsm

#
# update plugins/dialogs/about/module.h
#
cat plugins/dialogs/about/module.h | awk -v newver=$NEW_VERSION '{ 
	split($0, a, " ") } {
	if ((a[1] == "#define") && (a[2] == "VERSION")) {
	    printf("#define VERSION \"%s\"\n", newver)
	} else 
	    print $0
	}' > /tmp/module.h.new
mv plugins/dialogs/about/module.h /tmp/module.h.old
mv /tmp/module.h.new plugins/dialogs/about/module.h


echo ", new version = $NEW_VERSION."

#
# end of file
#
