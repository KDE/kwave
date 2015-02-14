#!/bin/sh
############################################################################
#   Messages.sh - script to extract translatable messages
#                            -------------------
#   begin                : Mon Jan 26 2015
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

# collect all messages from the .ui files
$EXTRACTRC `find . -name \*.ui | sort` >> rc.cpp

# fetch all messages from the C++ source and header files
$XGETTEXT `find . -name \*.cpp -o -name \*.h | sort` \
        -o $podir/kwave.pot

# create a pot file from menus.config
perl bin/menusconfig2pot.pl kwave/menus.config _kwave-i18n-menu.pot

# put all parts together
msgcat $podir/kwave.pot _kwave-i18n-menu.pot -o $podir/kwave.pot

# clean up
rm -f _kwave-i18n* rc.cpp

### EOF ###
