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

for file in `cd $1 ; find . -name \*.png | sed s+^\.\/++g `; do {
    echo ${file}
    convert $1/${file} -dither none -colors 63 doc/${file}
} done

#
# end of file
#
