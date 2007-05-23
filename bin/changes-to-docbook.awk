############################################################################
#   changes-to-docbook.awk - script to convert the CHANGES file of kwave
#                          into docbook format
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
# cat CHANGES | awk -f changes-to-docbook.sh > changes.docbook

BEGIN {
    initial_indent=1;
    n_versions=0;
    n_items=0;
    printi(0,"<para><itemizedlist>");
}
function printi(indent,line) {
    for (i=0;i<indent+initial_indent;i++) {
	printf("    ");
    }
    print line;
}
function print_version(ver) {
    printi(2,"<para>"ver"</para>");
}
{
    if (match($0, "^[[:digit:]]") == 1) {
	#
	# version headline
	#
	line=$0;
	if (n_subitems != 0) {
	    printi(4,"</para></listitem>");
	    printi(3,"</itemizedlist>");
	    n_subitems=0;
	}
	if (n_items != 0) {
	    printi(3,"</para></listitem>");
	    printi(2,"</itemizedlist>");
	    in_version=0;
	}

	if (n_versions != 0) printi(1,"</listitem>");

	printi(1,"<listitem>");
	print_version(line);
	n_versions++;
	n_items=0;
    } else if (match($0, "^\\ \\*\\ ") == 1) {
        #
	# changed item
	#
	if (n_subitems != 0) {
	    printi(4,"</para></listitem>");
	    printi(3,"</itemizedlist>");
	    n_subitems=0;
	}
	line=substr($0,4);
	if (n_items == 0) printi(2,"<itemizedlist>");
	else printi(3,"</para></listitem>");
	n_items++;
	printi(3,"<listitem><para>");
	printi(4,line);
    } else if (match($0, "\\ \\ \\ -\\ ") == 1) {
	#
	# sub-item
	#
	line=substr($0,5);
	if (n_subitems == 0) printi(4,"<itemizedlist>")
	else printi(4,"</para></listitem>");
	n_subitems++;
	printi(4,"<listitem><para>");
	printi(5,line);
    } else {
	#
	# any other line
	#
	sub("\\ ", "", $0);
	if (length($0) != 0) printi(4,$0);
    }
}
END {
    if (n_subitems != 0) {
	printi(4,"</para></listitem>");
	printi(3,"</itemizedlist>");
    }
    if (n_items != 0) {
	printi(3,"</para></listitem>");
	printi(2,"</itemizedlist>");
    }
    if (n_versions != 0) printi(1,"</listitem>");

    printi(0,"</itemizedlist></para>");
}