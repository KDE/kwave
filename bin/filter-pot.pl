#!/usr/bin/perl
#############################################################################
##    Kwave                - filter-pot.pl
##                           -------------------
##    begin                : Tue Jan 06 2015
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

# split argument string
my $in      = $ARGV[0];
my $out     = $ARGV[1];

open(IN,  "<" . $in ) or die "cannot open input file "  . $in;
open(OUT, ">" . $out) or die "cannot open output file " . $out;

my $last_line;
my $mode="comment";
my $filtered = 0;
my @msgid;

LINE: while (<IN>) {
    my $line = $_;
    chomp $line;

    if (!$line || ($line =~ /^#/)) {
	# start of a comment block or empty line
	$mode = "comment";
	$filtered = 0;

	# filter out useless duplicated comment lines
	next LINE if ($line eq $last_line);
    } elsif ($line =~ /^msgstr\ \"(.*)\"/) {
	# msgstr, might be filtered or not
	$mode = "msgstr";
    } elsif ($line =~ /^msgid\ (\".*\")/) {
	# start of a new message id
	local $id = $1;
	$mode = "msgid";
	push(@msgid, $id);
	$last_line=$line;
	next LINE;
    }

    if ($mode eq "msgid") {
	# still within a multi line msgid
	push(@msgid, $line);
	$last_line=$line;
	next LINE;
    }

    if (@msgid && ! ($mode eq "msgid")) {
	# change from msgid to something else
	$filtered = 0;
	foreach (@msgid) {
	    my $id= $_;
	    $filtered = 1 if ($id =~ /^\"&#x00[\dABCDEF]{2}\;\"$/);
	    $filtered = 1 if ($id =~ /^\"\<literal\>\%[\dABCDEF]{2}\<\/literal\>\"$/);
	    $filtered = 1 if ($id =~ /^\"&no\-i18n\-[\w_]+\;\"$/);
	    $filtered = 1 if ($id =~ /&no\-i18n\-tag\;/);
	    last if $filtered;
	}
	local $prefix = "";
	$prefix = "# (filtered) " if $filtered;
	print OUT $prefix . "msgid ";
	local $first = 1;
	foreach (@msgid) {
	    print OUT $prefix if (!$first);
	    print OUT $_ . "\n";
	    $first = 0;
	}
	@msgid=();
    }

    print OUT "# (filtered) " if $filtered;
    print OUT $line . "\n";
    $last_line=$line;
}

close(IN);
close(OUT);

#############################################################################
#############################################################################
