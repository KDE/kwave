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

use I18N::LangTags::List;

my $top_dir  = $ARGV[0] . "/po";
my $app      = "kwave";

sub check_po
{
    local $name  = shift;
    local $lang  = shift;
    local $scope = shift;
    local $file  = shift;

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
	    "| %-22s | %-11s | %s   |    %9s |    %9s |    %9s |",
	    $name, $lang, $scope, $translated, $fuzzy, $untranslated
	);

	print " <= " if (($fuzzy + $untranslated) == 0);

	print "\n";
    }

}

opendir(DIR, $top_dir) or die "opendir $top_dir: $!";
local @dirs = grep { !/(^\.{1,2}$)|(\.git)/ } readdir(DIR);
closedir(DIR);

local @catalogs;
foreach $entry (@dirs)
{
	local $e = $top_dir . "/" . $entry;
	if (-d $e)
	{
	    my $catalog = $entry;
	    push(@catalogs, $catalog) if (not grep { /$catalog/ } @catalogs);
	}
	elsif ($entry =~ /(.*)\.po/)
	{
	    my $catalog = $entry;
	    push(@catalogs, $catalog) if (not grep { /$catalog/ } @catalogs);
	}
}

print "+------------------------+-------------+-------+--------------+--------------+--------------+\n";
print "| language name          | lang        | scope |   translated |        fuzzy | untranslated |\n";
print "+------------------------+-------------+-------+--------------+--------------+--------------+\n";

foreach $catalog (@catalogs)
{
    if ($catalog =~ /(..)[\_\@]*/) {
	local $lang=$1;
	local $lang_name = I18N::LangTags::List::name($lang);
	local $po_gui=$top_dir . "/" . $catalog . "/" . $app . ".po";

	check_po($lang_name, $catalog, "GUI", $po_gui);

	if ((-e $top_dir . "/" . $po_doc) || (-e $top_dir . "/" . $po_gui) || (-e $top_dir . "/" . $po_dsk)) {
	    print "+------------------------+-------------+-------+--------------+--------------+--------------+\n";
	}
    }
}

#
# end of file
#
