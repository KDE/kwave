#!/bin/sh
#
# make_patch - script to make a new release of the amr project
# 
# 09.02.1999 by Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
#
# This script temporarily unpacks the previous version of the project 
# in /tmp, makes a gzipped patch file and increases the VERSION
# variable of the Makefile in the current directory.
#
# All version numbers consist of major.minor.revision (e.g. 0.4.2), the
# archived files are made out of the project name, followed by a "-" and
# the version number. It is assumed that the extension of the archives
# is ".tgz" or ".tar.gz".
#
# parameters:
# $1 name of the project (amr)
# $2 actual version number (0.4.2)
# $3 directory of the archives (/home/the/Diplomarbeit/Archiv)
# $4 source directory of the project (/home/the/Diplomarbeit/src)
# $5 prefix within the archive that should be skipped (src)
#
# NOTE: this should be regarded to be a quick hack, only few error checking 
#       is performed !!!
#
# 25.12.1999: donated this script to the kwave project and renamed it
#             from make_release.sh to make_patch.sh
#             moved the version number increment stuff to the
#             make_release and make_patchlevel scripts
#

# uncomment this for debugging
# set -x

PROJECT=$1
VERSION=$2
ARCHIVE_DIR=`cd $3; pwd`
SRC_DIR=$4
SRC_PREFIX=$5

RETVAL=""

#
# expands a version number into four parts (major, minor, release, patchlevel),
# gives them leading zeroes so that they get three digits long and returns
# the result as one string in the variable RETVAL.
#
function expand_version_number() {
    MAJOR=`echo $1 | \
	awk '{ split($0, a, ".") } END { printf "%03d", a[1] }'`
    MINOR=`echo $1 | \
	awk '{ split($0, a, ".") } END { printf "%03d", a[2] }'`
    RELEASE=`echo $1 | \
	awk '{ split($0, a, ".") } END { printf a[3] }'`
    PATCHLEVEL=`echo $1 | \
		awk '{ split($0, a, "-") } END { print a[2] }'`

    if test ! -z "$PATCHLEVEL"; then
	PATCHLEVEL=`echo $RELEASE | \
		awk '{ split($0, a, "-") } END { printf "%03d", a[2] }'`
	RELEASE=`echo $RELEASE | \
		awk '{ split($0, a, "-") } END { printf "%03d", a[1] }'`
    else
	PATCHLEVEL="000"
	RELEASE=`echo $RELEASE | \
			awk '{ split($0,a,"-") } END { printf "%03d", a[1] }'`
    fi

    export RETVAL="$MAJOR"."$MINOR"."$RELEASE"-"$PATCHLEVEL"
    return 0
}

echo "making a new patch..."
echo -e "\tproject "$PROJECT", ver" $VERSION
echo -e "\tarchives in "$ARCHIVE_DIR

if test ! -e "$ARCHIVE_DIR"; then
    mkdir -p "$ARCHIVE_DIR"
fi
if test ! -e "$ARCHIVE_DIR"; then
    echo -e "\tarchive directory not found and cannot be created :-("
    exit 1
fi

#
# get the list of archive files
#
ARCHIVES=`ls $ARCHIVE_DIR/*.tar.gz | grep ^$ARCHIVE_DIR/$PROJECT-`

#
# get the actual version number
#
ACT_REL=`echo $VERSION"." | \
    awk '{ split($0,a,".") } END { printf "%s.%s.%s", a[1],a[2],a[3] }'`
expand_version_number $ACT_REL
ACT_VERSION=$RETVAL
# echo -e "\tactual version number (expanded): "$ACT_VERSION

#
# search for the previous version
#

# last version numbers (maximum, but less than actual)
EXPANDED_PREV_VERSION=000.000.000-000
PREV_FILE=""
PREV_VERSION=""
for archive in $ARCHIVES; do
{
    #
    # get the version number of the archive
    #
    ARCH_FILE=${archive/\#$ARCHIVE_DIR/""}
    ARCH_FILE=${ARCH_FILE/\#\/$PROJECT-/""}
    # echo ARCH_FILE=\"$ARCH_FILE\"

    ARCH_VERSION=`echo $ARCH_FILE"." | \
	awk '{ split($0,a,".") } END { printf "%s.%s.%s", a[1],a[2],a[3] }'`
    # echo ARCH_VERSION=\"$ARCH_VERSION\"

    expand_version_number $ARCH_VERSION
    EXPANDED_ARCH_VERSION=$RETVAL
    # echo -e "\tarchive version number (expanded): "$EXPANDED_ARCH_VERSION

    #
    # compare and store the nearest version
    #
    if test $EXPANDED_ARCH_VERSION \> $EXPANDED_PREV_VERSION; then
	if test "$EXPANDED_ARCH_VERSION" \< "$ACT_VERSION"; then
	    export EXPANDED_PREV_VERSION=$EXPANDED_ARCH_VERSION
	    export PREV_FILE=$ARCH_FILE
	    export PREV_VERSION=$ARCH_VERSION
	    # echo "ARCH="$ARCH_VERSION", ACT="$ACT_VERSION
	fi
    fi

};
done

# echo PREV_VERSION=\"$PREV_VERSION\"
# echo PREV_FILE=\"$PREV_FILE\"

if test -z "$PREV_FILE"; then
    echo -e "\tno previous version found, nothing to do :-("
    exit 1
fi

#
# unpack the previous version in /tmp
#
cd /tmp
if test -e $PROJECT-$PREV_VERSION; then
    echo -e "\tremoving "$PROJECT-$PREV_VERSION
    rm -Rf $PROJECT-$PREV_VERSION
fi

echo -e "\tunpacking previous version in "/tmp/$PROJECT-$PREV_VERSION
mkdir $PROJECT-$PREV_VERSION
cd -
cd /tmp/$PROJECT-$PREV_VERSION
tar -xzf $ARCHIVE_DIR/$PROJECT-$PREV_FILE
cd -

#
# generate the patch
#
DIFF_ROOT=${SRC_DIR:0:$((${#SRC_DIR}-${#SRC_PREFIX}-1))}
PATCH_FILE=$ARCHIVE_DIR/$PROJECT-$VERSION.diff.gz
echo -e "\tpatch file="$PATCH_FILE
echo -e "\tpatch root="$DIFF_ROOT
echo -e "\t\told="/tmp/$PROJECT-$PREV_VERSION
echo -e "\t\tnew="$DIFF_ROOT"/"$SRC_PREFIX
cd $DIFF_ROOT
diff -Naur /tmp/$PROJECT-$PREV_VERSION \
		       $SRC_PREFIX \
		       | sed s°^---\ /tmp/°---\ °g \
		       | sed s°^diff\ -Naur\ /tmp/°diff\ -Naur\ °g \
		       | gzip -9 - > $PATCH_FILE
cd -

#
# remove the temporary directory
#
echo -e "\tremoving temporary directory "$PROJECT-$PREV_VERSION
rm -Rf /tmp/$PROJECT-$PREV_VERSION

echo "patch created."
