#!/bin/sh
############################################################################
#   insert-changes - inserts a CHANGES file into a docbook file
#                            -------------------
#   begin                : Wed Jun 21 2000
#   copyright            : (C) 2000 by Thomas Eschenbacher
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
# example for usage:
# cat help_en.orig | insert-changes.sh changes.docbook > help_en.docbook

# set -x

awk -v changes_file=$1 '
/<!--\ AUTO_GENERATED_CHANGES_LIST_START\ -->/ {
    start_tag_found=1;
    end_tag_found=0;
}

/<!--\ AUTO_GENERATED_CHANGES_LIST_END\ -->/ {
    start_tag_found=0;
    end_tag_found=1;
}

{
    if (start_tag_found==1 && ignore==0) {
	# ignore everything until end tag found
	print;
	ignore=1;
    } else if (end_tag_found==1) {
	# insert the new version
        while ((getline line < changes_file) > 0)
	    print line;
	close(changes_file);

	# set the end tag
	ignore=0;
	end_tag_found=0;
    }

    if (ignore==0) print;
}
'



