#!/bin/sh
#
# set_release - script to set version numbers of a project
# 
# 20.02.1999 by Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de> (THE)
#
# This script get the new version number via commandline
#
# All version numbers consist of major.minor.revision (e.g. 0.4.2) Only the 
# last number, the release will be incremented. Any suffix (patchlevel) 
# will be removed.
#
# the updated files are: 
# - VERSION    (the file itself)
# - configure
# - Makefile (if it exists)
# - libkwave.lsm

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
NEW_VERSION=$1

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
# update the file libkwave.lsm
#
DATE=`date`
cat libkwave.lsm | awk -v newver=$NEW_VERSION -v newdate=$DATE '{ 
	split($0, a, ":") } {
	if (a[1] == "Version") {
	    printf("Version:\t%s\n", newver)
	} else
	if (a[1] == "Entered-date") {
	    printf("Entered-date:\t%s\n", newdate)
	} else 
	    print $0
	}' > /tmp/libkwave.lsm.new
mv libkwave.lsm /tmp/libkwave.lsm.old
mv /tmp/libkwave.lsm.new libkwave.lsm

echo ", new version = $NEW_VERSION."

#
# end of file
#
