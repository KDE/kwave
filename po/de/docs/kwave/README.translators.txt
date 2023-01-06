#############################################################################
##    doc/en/README.translators.txt - notes for translators
##                           -------------------
##    begin                : Sat Feb 21 2015
##    copyright            : (C) 2015 by Thomas Eschenbacher
##    email                : Thomas.Eschenbacher@gmx.de
#############################################################################
#
#############################################################################
##                                                                          #
##    This program is free software; you can redistribute it and/or modify  #
##    it under the terms of the GNU General Public License as published by  #
##    the Free Software Foundation; either version 2 of the License, or     #
##    (at your option) any later version.                                   #
##                                                                          #
#############################################################################

Dear translators,

this directory contains many screenshots of Kwave. Please don't waste your
precious spare time in making these manually!

You can do that if you want, by doing the following steps:

1) Adjust your KDE desktop settings according to the guidelines specified in
   http://l10n.kde.org/docs/screenshots.php, and don't forget to set the
   "contrast" setting of the color scheme to zero (produces smaller files)

2) Set your desktop to the desired language, or alternatively set the
   environment variables KDE_LANG and LC_ALL. Here the example for cs_CZ:

       export KDE_LANG=cs_CZ.utf8
       export LC_ALL=cs

3) create an empty directory /var/tmp/screenshots/${LC_ALL}

       mkdir -p /var/tmp/screenshots/${LC_ALL}

4) Start kwave and load "scripts/screenshots.kwave".
   This script automatically creates screenshots for the desired language.

       cd <directory with kwave sources>
       kwave scripts/screenshots.kwave

5) Import the screenshots into your Kwave source tree through the script
   bin/import-screenshots.sh, which does some dithering and color space
   reduction to reduce the file size of the images:

       mkdir -p doc/${LC_ALL}
       bin/import-screenshots.sh /var/tmp/screenshots/${LC_ALL}

### EOF ###
