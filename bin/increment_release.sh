#!/bin/sh
#
# increment_release - script to set new version numbers of a project
# 
# 20.02.1999 by Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de> (THE)
#
# parameters:
# $1 = the project root directory
#
# All version numbers consist of major.minor.revision (e.g. 0.4.2) Only the 
# last number, the release will be incremented. Any suffix (patchlevel) 
# will be removed.
#
# This script will call the "set_version" script to feed the changes into
# various other files.
#
# NOTE: - this should be considered a quick hack, no error 
#         checking is performed !!!
#
# 26.12.1999 THE, adapted the script to work for the kwave project

# uncomment the next line for debugging
# set -x

cd $1

#
# get the current version number and the actual date/time
#
VERSION=`cat ./VERSION`
DATE="`date --iso`"

echo "   old version number : \"$VERSION"\"

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

#
# call the "set_version" script in order to feed the new version number
# and the date in all necessary files
#
bin/set_version.sh $1 $NEW_VERSION "$DATE"

#
# end of file
#
