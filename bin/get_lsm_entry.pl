#!/usr/bin/perl
############################################################################
#   get_lsm_entry.pl - gets the value of an entry in an lsm file
#                            -------------------
#   begin                : Sun Dec 21 2014
#   copyright            : (C) 2014 by Thomas Eschenbacher
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

open (IN,  "<$ARGV[0]") or die("open input file failed");
my $entry = $ARGV[1];

while (<IN>) {
    my $line = $_;

    $line =~ s/\s+$//;
    $line =~ s/^\s+|\s+$//g;

    $line =~ /^($entry)(:\s*)(.*)(\s*$)/;
    if ($1 eq $entry) {
	print $3 . "\n";
	exit 0;
    }
}

exit -1;

### EOF ###
