#!/bin/sh
#
# increment_patchlevel - script to set a new patchlevel of a project
# 
# 20.02.1999 by Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de> (THE)
#
# parameters:
# $1 = the project root directory
#
# All version numbers consist of major.minor.revision (e.g. 0.4.2) and an 
# additional patchlevel. Only the patchlevel will be incremented by this
# script, the rest will be remain untouched. If currently no patchlevel
# exisits it will be created, starting from "-1".
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
# get the actual version number == new one
#
NEW_MAJOR=`echo $VERSION | \
		awk '{ split($0, a, ".") } END { print a[1] }'`
NEW_MINOR=`echo $VERSION | \
		awk '{ split($0, a, ".") } END { print a[2] }'`
NEW_RELEASE=`echo $VERSION | \
		awk '{ split($0, a, ".") } END { print a[3] }'`

#
# try to get the patchlevel
#
NEW_PATCHLEVEL=`echo $VERSION | \
		awk '{ split($0, a, "-") } END { print a[2] }'`

#
# if one exists, it is also accidently contained in the release, so
# remove it and increment the patchlevel
#
if test ! -z "$NEW_PATCHLEVEL"; then
    NEW_PATCHLEVEL=`echo $NEW_RELEASE | \
		    awk '{ split($0, a, "-") } END { print a[2]+1 }'`
    NEW_RELEASE=`echo $NEW_RELEASE | \
		    awk '{ split($0, a, "-") } END { print a[1] }'`
else
    # starting a new patch serie from one
    NEW_PATCHLEVEL="1"
fi

#
# construct the new version number
#
NEW_VERSION=$NEW_MAJOR"."$NEW_MINOR"."$NEW_RELEASE"-"$NEW_PATCHLEVEL

#
# call the "set_version" script in order to feed the new version number
# and the date in all necessary files
#
bin/set_version.sh $1 $NEW_VERSION "$DATE"

#
# end of file
#
