#!/usr/bin/perl
############################################################################
#   msgstats.pl - script to show the current progress of the translations
#                            -------------------
#   begin                : Tue Oct 27 2015
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
# $1 = project root directory
#

my $top_dir  = $ARGV[0] . "/l10n-kf5";
my $category = "kdereview";
my $app      = "kwave";

sub check_po
{
    local $name  = shift;
    local $lang  = shift;
    local $scope = shift;
    local $file  = $top_dir . "/" . shift;

    local $translated   = 0;
    local $fuzzy        = 0;
    local $untranslated = 0;

    if ( -e $file) {
	local $state = `LC_ALL=C msgfmt --statistics $file 2>&1`;
	if ($state =~ m/(\d+)\ translated/) {
	    $translated = $1;
	}
	if ($state =~ m/(\d+)\ fuzzy/) {
	    $fuzzy = $1;
	}
	if ($state =~ m/(\d+)\ untranslated/) {
	    $untranslated = $1;
	}
    }

    if (($translated + $fuzzy + $untranslated) != 0) {
	printf(
	    "| %-22s | %-5s | %s   |    %9s |    %9s |    %9s |",
	    $name, $lang, $scope, $translated, $fuzzy, $untranslated
	);

	print " <= " if (($fuzzy + $untranslated) == 0);

	print "\n";
    }

}

open (IN, $top_dir . "/teamnames") or die("open input file failed");

print "+------------------------+-------+-------+--------------+--------------+--------------+\n";
print "| language name          | lang  | scope |   translated |        fuzzy | untranslated |\n";
print "+------------------------+-------+-------+--------------+--------------+--------------+\n";

while (<IN>) {
    my $line = $_;
    if ($line =~ /(.*)=(.*)/) {
	local $catalog=$1;
	local $lang_name=$2;
	# print $lang_name . " [" . $catalog . "]\n";

	local $po_doc=$catalog . "/docmessages/" . $category . "/" . $app . ".po";
	local $po_gui=$catalog . "/messages/" . $category . "/" . $app . ".po";

	check_po($lang_name, $catalog, "DOC", $po_doc);
	check_po($lang_name, $catalog, "GUI", $po_gui);

	if ((-e $top_dir . "/" . $po_doc) || (-e $top_dir . "/" . $po_gui)) {
	    print "+------------------------+-------+-------+--------------+--------------+--------------+\n";
	}

    }
}
close IN;

#
# end of file
#
