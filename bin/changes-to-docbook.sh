#!/bin/sh
#
# changes-to-docbook.sh - script to convert the CHANGES file of kwave
#                         into docbook format
#
# 2000-06-21 by Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de> (THE)
#
# example for usage:
# cat CHANGES | changes-to-docbook.sh > changes.docbook

# set -x

awk '
BEGIN { 
    initial_indent=1;
    n_versions=0;
    n_items=0;
    printi(0,"<PARA><ITEMIZEDLIST>");
}
function printi(indent,line) {
    for (i=0;i<indent+initial_indent;i++) {
	printf("    ");
    }
    print line;
}
function print_version(ver) {
    printi(2,"<PARA>"ver"</PARA>");
}
{ 
    if (match($0, "^[[:digit:]]") == 1) {
	#
	# version headline
	#
	line=$0;
	if (n_subitems != 0) {
	    printi(4,"</PARA></LISTITEM>");
	    printi(3,"</ITEMIZEDLIST>");
	    n_subitems=0;
	}
	if (n_items != 0) {
	    printi(3,"</PARA></LISTITEM>");
	    printi(2,"</ITEMIZEDLIST>");
	    in_version=0;
	}

	if (n_versions != 0) printi(1,"</LISTITEM>");

	printi(1,"<LISTITEM>");
	print_version(line);
	n_versions++;
	n_items=0;
    } else if (match($0, "^\\ \\*\\ ") == 1) {
        #
	# changed item
	#
	if (n_subitems != 0) {
	    printi(4,"</PARA></LISTITEM>");
	    printi(3,"</ITEMIZEDLIST>");
	    n_subitems=0;
	}
	line=substr($0,4);
	if (n_items == 0) printi(2,"<ITEMIZEDLIST>");
	else printi(3,"</PARA></LISTITEM>");
	n_items++;
	printi(3,"<LISTITEM><PARA>");
	printi(4,line);
    } else if (match($0, "\\ \\ \\ -\ ") == 1) {
	#
	# sub-item
	#
	line=substr($0,5);
	if (n_subitems == 0) printi(4,"<ITEMIZEDLIST>")
	else printi(4,"</PARA></LISTITEM>");
	n_subitems++;
	printi(4,"<LISTITEM><PARA>");
	printi(5,line);
    } else {
	#
	# any other line
	#
	sub("\ ", "", $0);
	if (length($0) != 0) printi(4,$0);
    }
}
END { 
    if (n_subitems != 0) {
	printi(4,"</PARA></LISTITEM>");
	printi(3,"</ITEMIZEDLIST>");
    }
    if (n_items != 0) {
	printi(3,"</PARA></LISTITEM>");
	printi(2,"</ITEMIZEDLIST>");
    }
    if (n_versions != 0) printi(1,"</LISTITEM>");

    printi(0,"</ITEMIZEDLIST></PARA>");
}
'
