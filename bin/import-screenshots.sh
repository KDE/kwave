#!/bin/bash
############################################################################
#   import-screenshots.sh - import and resize/shrink screenshots to doc/...
#                            -------------------
#   begin                : Fri Jan 23 2015
#   copyright            : (C) 2015 by Thomas Eschenbacher
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
# $1 = directory where the screenshots are stored
#

# example:
# find /var/tmp/screenshots -maxdepth 1 -mindepth 1 -type d -exec bin/import-screenshots.sh \{\} \;

LANG=`basename $1`
DEST=l10n-kf5/${LANG}/docs/kdereview/kwave
if test ! -e l10n-kf5/${LANG} ; then
    LANG=`echo ${LANG} | cut -d _ -f 1`
    DEST=l10n-kf5/${LANG}/docs/kdereview/kwave
fi

OPTIONS="-dither none -colors 63"

echo "importing screenshots into ${DEST}..."

# flatten the three layers of the SDI mode screenshots
# into one single image
echo kwave-gui-sdi.png
convert -size 906x667 xc:transparent -background transparent \
	-page +40+0   "$1/01-kwave-gui-sdi.png" \
	-page +80+80  "$1/02-kwave-gui-sdi.png" \
	-page  +0+200 "$1/03-kwave-gui-sdi.png" \
	-layers flatten \
	"$1/kwave-gui-sdi.png"

mkdir -p ${DEST}

for file in `cd $1 ; find . -name kwave-\*.png | sed s+^\.\/++g `; do {
	echo ${file}
	convert $1/${file} ${OPTIONS} ${DEST}/${file}
} done

#
# end of file
#
