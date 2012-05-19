#!/usr/bin/perl
#############################################################################
##    Kwave                - make-specfile-changelog.pl
##                           -------------------
##    begin                : Sun Nov 13 2011
##    copyright            : (C) 2011 by Thomas Eschenbacher
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

use Time::Local;
use Date::Format;

# split argument string
my $in = $ARGV[0];
my $out = $ARGV[1];

my $default_author = "Thomas Eschenbacher <Thomas.Eschenbacher\@gmx.de>";

open(IN,  "<" . $in ) or die "cannot open input file "  . $in;
open(OUT, ">" . $out) or die "cannot open output file " . $out;

while (<IN>) {

    $a = $_;
    chomp($a);

    # remove trailing whitespaces
    $string =~ s/\s+$//;

    # if the line has the format <version> [ "[" <date> "]" ], then it
    # is the start of a new package version
    if ($a =~ /^([\.\-\d]+)\s+((\[.*\])|(\(.*\)))?/) {
	my $version = $1;
	my $description = $2;
	my $date = "";
	my $author = $default_author;

	# if author is given with (<author>) take it, otherwise use default
	if ($a =~ /\((.*)\)/) {
	    $author = $1; # different author
	}

	# if the version is an ISO date, use it as date instead, no version
	if ($version =~ /\d\d\d\d\-\d\d\-\d\d/) {
	    $description = "[" . $version . "]";
	    $version = "";
	}

	# prepend a "v" in front of version numbers
	if ($version =~ /\d+\.\d+[\.\d+]?[\-\d+]?/) {
	    $version = "v" . $version;
	}

	# if description contains a ISO date, take it
	if ($description =~ /\[(\d\d\d\d)\-(\d\d)\-(\d\d)\]/) {
	    $timestamp = timelocal(0, 0, 0, $3, $2 - 1, $1);
	    $date = time2str("%a %b %e %Y", $timestamp);;
	}

	print OUT "\n";
	print OUT "-------------------------------------------------------------------\n";
	print OUT $date . " - " . $author . "\n";
	print OUT "\n";
	if ($version) {
	    print OUT "- " . $version . "\n";
	}
    }
    elsif ($a)
    {
	# expand leading tabs to spaces

	# remove first space
	$a =~s+^\ ++g;

	# replace intended bullets '-' -> '*'
	$a =~ s+^\ \ \-\ +\ \ \*\ +g;

	# replace leading bullets '*' -> '-'
	$a =~ s+^\*+\-+g;

	print OUT $a . "\n";
    }

}
print OUT "\n";

close(IN);
close(OUT);

#############################################################################
#############################################################################
