#!/usr/bin/perl
#############################################################################
##    Kwave                - update-fileinfo-xref.pl
##                           -------------------
##    begin                : Fri Apr 03 2015
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
my $src     = $ARGV[1];
my $out     = $ARGV[2];

open(IN,  "<" . $in ) or die "cannot open input file "  . $in;
open(OUT, ">" . $out) or die "cannot open output file " . $out;

my @scanned_properties;

sub scan_file
{
    local $file = shift;
    # print "scanning file: '" . $file . "'\n";

    open(IN2, $file);
    local $old_mode = $/;

    local @lines;
    local $quoted = 0;
    $/ = "";
    while (<IN2>) {
	local $data = $_;
	local $line;
	for (my $i = 0; $i < length($data); $i += 1) {
	    my $c = substr($data, $i, 1);
	    if ($quoted and ($c eq "\"")) {
		$quoted = 0;
	    } elsif ((not $quoted) and ($c eq "\"")) {
		$quoted = 1;
	    }
	    if (not $quoted and ($c eq ";")) {
		push(@lines, $line);
# 		print "line='" . $line . "'\n";
		$line = "";
	    } else {
		$line = $line . $c;
	    }
	}
	push(@lines, $line) if ($line);
    }

    LINE: foreach (@lines) {
	local $line = $_;
	# parse the complete append(...) expression, up to the end
	if ($line =~ m/append\s*/) {
	    # split by comma
	    local @params;
	    local $call   = $';
	    local $param;
	    local $p = '"';
	    for (my $i = 0; $i < length($call); $i += 1) {
		my $c = substr($call, $i, 1);
		if ($quoted and ($c eq "\"") and not ($p eq '\\')) {
		    $quoted = 0;
		} elsif ((not $quoted) and ($c eq "\"") and not ($p eq '\\')) {
		    $quoted = 1;
		}
		if (not $quoted and ($c eq ",")) {
		    push(@params, $param);
		    $param = "";
		} else {
		    $param = $param . $c;
		}
		$p = $c;
	    }
	    push(@params, $param) if ($param);

	    # take out the parameters
	    local $property    = @params[0];
	    local $flags       = @params[1];
	    local $name        = @params[2];
	    local $description = @params[3];

	    # postprocessing
	    $property    =~ m/[\s\n]*Kwave::([\S\s]+)[\s\n]*/; $property    = $1;
	    $flags       =~ m/[\s\n]*([\S\s\|.]+)[\s\n]*/;     $flags       = $1;
	    $name        =~ m/[\s\n]*([\S\s]+)[\s\n]*/;        $name        = $1;
	    $description =~ m/[\s\n]*([\S\s]+)[\s\n]*/;        $description = $1;

	    if ($name =~ m/\_\(I18N_NOOP\(\"([\S\s\(\)\"\,]+)\"\)\)/) { $name = $1; }

	    $description =~ s/^\_\(I18N_NOOP\(//g;
	    $description =~ s/^\s*\"//g;
	    $description =~ s/\"\s*$//g;
	    $description =~ s/\"\n\s*\"//g;
	    $description =~ s/\n\s*\"/\ /g;
	    $description =~ s/\\n/\ /g;
	    $description =~ s/\"[\)]*$//g;
	    $description =~ s/\&/&amp;/g;
	    $description =~ s/\\\"/&quot;/g;
	    $description =~ s/\'/&quot;/g;

	    next LINE if (! length($name));
	    next LINE if ($property eq "INF_UNKNOWN");

# 	    print "'"    . $property;
# 	    print "', '" . $flags;
# 	    print "', '" . $name;
# 	    print "', '" . $description;
# 	    print "'\n";

	    push(@scanned_properties, {
		property    => $property,
		flags       => $flags,
		name        => $name,
		description => $description
	    }) if ((! grep {$_->{property} eq $property} @scanned_properties)
	           and not ($name eq "QString()"));
	}
    }
    close(IN2);
    $/ = $old_mode;
}

#
# scan the list of all plugins
#
scan_file($src);

my $mode = "normal";

LINE: while (<IN>) {
    my $line = $_;
    $linenr += 1;

    if ($mode eq "normal") {
	if ($line =~ /\<\!\-\-\ \@FILEINFO_ENTITIES_START\@\ \-\-\>/) {
	    print OUT $line;
	    $mode = "entities";
	    next LINE;
	}

	if ($line =~ /\<\!\-\-\ \@FILEINFO_TABLE_START\@\ \-\-\>/) {
	    print OUT $line;
	    $mode = "table";
	    next LINE;
	}
    }

    if ($line =~ /\<\!\-\-\ \@FILEINFO_ENTITIES_END\@\ \-\-\>/) {
	foreach (@scanned_properties) {
	    local $property = $_->{property};
	    local $name     = $_->{name};
	    $property =~ s/[^\w]/_/g;
	    print OUT "  \<\!ENTITY i18n-" . $property . " \"" . $name . "\"\>\n";
	}
	print OUT $line;
	$mode = "normal";
	next LINE;
    }

    if ($line =~ /\<\!\-\-\ \@FILEINFO_TABLE_END\@\ \-\-\>/) {
	foreach (@scanned_properties) {
	    local $property = $_->{property};
	    local $name     = $_->{name};
	    local $description = $_->{description};

	    print OUT "\t<row id=\"" . $property . "\">\n";
	    print OUT "\t    <entry colname='c1'>&no-i18n-tag;". $name . "</entry>\n";
	    print OUT "\t    <entry colname='c2'>\n";
	    while (length($description)) {
		$part = $description;
		if (length($description) >= (79 - 16)) {
		    local $i = 79 - 16;
		    while (($i > 1) and (substr($description, $i, 1) !~ /\s/)) {
			$i = $i - 1;
		    }
		    $part = substr($description, 0, $i);
		    chomp $part;
		}
		print OUT "\t\t" . $part . "\n";
		$description = substr($description, length($part));
		chomp $description;
	    }
	    print OUT "\t    </entry>\n";
	    print OUT "\t</row>\n";
	}
	print OUT $line;
	$mode = "normal";
	next LINE;
    }

    print OUT $line if ($mode eq "normal");
}

close(IN);
close(OUT);

#############################################################################
#############################################################################
