#!/usr/bin/perl
#############################################################################
##    Kwave                - update-plugin-xref.pl
##                           -------------------
##    begin                : Sat Mar 28 2015
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
my $src_dir = $ARGV[0];
my $in      = $ARGV[1];
my $out     = $ARGV[2];

open(IN,  "<" . $in ) or die "cannot open input file "  . $in;
open(OUT, ">" . $out) or die "cannot open output file " . $out;

my @scanned_plugins;

sub scan_file
{
    local $file = shift;
#     print "scanning file: '" . $file . "'\n";

    local $name;
    local $version;
    local $description;
    local $author;

    open(IN2, $file);
    while (<IN2>) {
	local $line = $_;
	chomp $line;

	$name        = $1 if ($line =~ /^Name=([\w]+)\s*/ );
        $version     = $1 if ($line =~ /^X-KDE-PluginInfo-Name=(\d+\.\d+)/ );
        $description = $1 if ($line =~ /^Comment=(.+)\s*/ );
        $author      = $1 if ($line =~ /^X-KDE-PluginInfo-Author=(.+)\s*/ );
    }
    close(IN2);

#     print "', '" . $name;
#     print "', '" . $version;
#     print "', '" . $description;
#     print "', '" . $author;
#     print "'\n";

    push(@scanned_plugins, {
	name        => $name,
	version     => $version,
	description => $description,
	author      => $author
    }) if (! grep {$_->{name} eq $name} @scanned_plugin);

}

sub scan_plugin_dir
{
    local $path = shift;

    opendir(DIR, $path) or die "opendir $path: $!";
    local @dirs = grep { !/(^\.)/ } readdir(DIR);
    closedir(DIR);
    @dirs = grep(!/common/, @dirs);
    @dirs = sort { $a cmp $b } @dirs;

    for (@dirs) {
	local $dir = $path . '/' . $_;
	if (-d $dir) {
	    opendir(SUBDIR, $dir) or die "opendir $dir: $!";
	    local @files = grep { !/(^\.)/ } readdir(SUBDIR);
	    closedir(SUBDIR);
	    @files = sort { $a cmp $b } @files;
	    for (@files) {
		local $file = $_;
		if ($file =~ /\.desktop.in$/) {
		    scan_file $dir . '/' . $file;
		}
	    }
	}
    }
}

sub make_stub
{
    local $plugin = shift;
    local $lbl = $plugin->{name};
    print OUT "    \<\!-- \@PLUGIN\@ " . $plugin->{name} . " (TODO) --\>\n";
    $lbl =~ s/[^\w]/_/g;

    print OUT "    <sect1 id=\"plugin_sect_" . $lbl . "\">" .
              "<title id=\"plugin_title_" . $lbl . "\">" .
	      "&no-i18n-plugin_" . $lbl . "; (" . $plugin->{description} . ")</title>\n";

    print OUT "    <!-- <screenshot>\n";
    print OUT "\t<screeninfo>Screenshot</screeninfo>\n";
    print OUT "\t<mediaobject>\n";
    print OUT "\t    <imageobject>\n";
    print OUT "\t\t<imagedata fileref=\"kwave-plugin-" . $lbl . ".png\" format=\"PNG\"/>\n";
    print OUT "\t    </imageobject>\n";
    print OUT "\t    <textobject>\n";
    print OUT "\t\t<phrase>Screenshot of the " . $plugin->{description} . " Plugin</phrase>\n";
    print OUT "\t    </textobject>\n";
    print OUT "\t</mediaobject>\n";
    print OUT "    </screenshot> -->\n";

    print OUT "    <variablelist>\n";
    print OUT "\t<varlistentry>\n";
    print OUT "\t    <term><emphasis role=\"bold\">&i18n-plugin_lbl_internal_name;</emphasis></term>\n";
    print OUT "\t    <listitem><para><literal>&no-i18n-plugin_" . $lbl . ";</literal></para></listitem>\n";
    print OUT "\t</varlistentry>\n";

    print OUT "\t<varlistentry>\n";
    print OUT "\t    <term><emphasis role=\"bold\">&i18n-plugin_lbl_type;</emphasis></term>\n";
    print OUT "\t    <listitem><para>effect|codec|encoder|decoder|gui|function</para></listitem>\n";
    print OUT "\t</varlistentry>\n";

    print OUT "\t<varlistentry>\n";
    print OUT "\t    <term><emphasis role=\"bold\">&i18n-plugin_lbl_description;</emphasis></term>\n";
    print OUT "\t    <listitem>\n";
    print OUT "\t    <para>\n";
    print OUT "\t        TODO: description of what the " . $plugin->{description} . " plugin does...\n";
    print OUT "\t    </para>\n";
    print OUT "\t    </listitem>\n";
    print OUT "\t</varlistentry>\n";

    print OUT "\t<!-- varlistentry>\n";
    print OUT "\t    <term><emphasis role=\"bold\">&i18n-plugin_lbl_parameters;</emphasis>:</term>\n";
    print OUT "\t    <listitem>\n";
    print OUT "\t\t<variablelist>\n";
    print OUT "\t\t    <varlistentry>\n";
    print OUT "\t\t\t<term><replaceable>operation</replaceable></term>\n";
    print OUT "\t\t\t<listitem>\n";
    print OUT "\t\t\t    <para>\n";
    print OUT "\t\t\t\tdescription of the parameter\n";
    print OUT "\t\t\t    </para>\n";
    print OUT "\t\t\t</listitem>\n";
    print OUT "\t\t    </varlistentry>\n";
    print OUT "\t\t</variablelist>\n";
    print OUT "\t    </listitem>\n";
    print OUT "\t</varlistentry -->\n";

    print OUT "    </variablelist>\n";
    print OUT "    </sect1>\n";
    print OUT "\n";
}

#
# scan the list of all plugins
#
scan_plugin_dir($src_dir);
my @remaining_plugins = @scanned_plugins;

my $mode = "normal";

LINE: while (<IN>) {
    my $line = $_;

    if ($mode eq "normal") {

	if ($line =~ /\<\!\-\-\ \@PLUGIN_INDEX_START\@\ \-\-\>/) {
	    print OUT $line;
	    $mode = "index";
	    next LINE;
	}

	if ($line =~ /\<\!\-\-\ \@PLUGIN_ENTITIES_START\@\ \-\-\>/) {
	    print OUT $line;
	    $mode = "entities";
	    next LINE;
	}

	if ($line =~ /\<\!\-\-\ \@PLUGIN\@\ ([\w\:]+)\ (\(TODO\)\ )?\-\-\>/) {
	    my $plugin = $1;
	    my $todo = $2;
	    # print "### found plugin '" . $plugin . "'\n";
	    if (not grep {$_->{name} eq $plugin} @scanned_plugins) {
		print STDERR "WARNING: plugin '" . $plugin . "' is not supported / does not exist\n";
	    } else {
		while (@remaining_plugins) {
		    my $next_plugin = shift @remaining_plugins;

		    # if plugin is next in list of remaining -> remove from list
		    # print "next_plugin=".$next_plugin.", plugin=".$plugin."\n";
		    if ($plugin eq $next_plugin->{name}) {
			# print "(FOUND)\n";
			# show a warning if there documentation is not finished
			# for this plugin
			if ($todo) {
			    print STDERR "WARNING: plugin '" . $plugin . "': documentation is not finished (TODO)\n";
			}
			print OUT $line;
			next LINE;
		    }

		    if ($plugin gt $next_plugin->{name}) {
			print STDERR "WARNING: plugin '" . $next_plugin->{name} . "' is undocumented, adding stub\n";
			make_stub $next_plugin;
		    } else {
			last;
		    }
		}
	    }
	    print OUT $line;
	    next LINE;
	}

	if ($line =~ /\<\!\-\-\ \@PLUGIN_END_OF_LIST\@\ \-\-\>/) {
	    # print "WARNING: some plugins left\n";
	    while (@remaining_plugins) {
		local $plugin = shift @remaining_plugins;
		print STDERR "WARNING: plugin '" . $plugin->{name} . "' is undocumented, adding stub\n";
		make_stub $plugin;
	    }
	}

	print OUT $line;
    }

    if ($line =~ /\<\!\-\-\ \@PLUGIN_ENTITIES_END\@\ \-\-\>/) {
	foreach (@scanned_plugins) {
	    local $plugin = $_->{name};
	    local $lbl = $plugin;
	    $lbl =~ s/[^\w]/_/g;
	    print OUT "  \<\!ENTITY no-i18n-plugin_" . $lbl . " \"" . $plugin . "\"\>\n";
	}
	print OUT $line;
	$mode = "normal";
	next LINE;
    }

    if ($mode eq "index") {
	if ($line =~ /\<\!\-\-\ \@PLUGIN_INDEX_END\@\ \-\-\>/) {

	    # create a new plugin index
	    my $first_char = "*";
	    foreach (@scanned_plugins) {
		my $plugin = $_->{name};
		my $first = substr($plugin, 0, 1);
		if (not ($first eq $first_char)) {
		    print OUT "\t    </indexdiv>\n" if (not ($first_char eq "*"));
		    print OUT "\t    <indexdiv><title>" . $first . "</title>\n";
		    $first_char = $first;
		}

		$plugin =~ s/[^\w]/_/g;
		print OUT "\t\t<indexentry><primaryie>" .
		    "<link linkend=\"plugin_sect_" . $plugin . "\" " .
		    "endterm=\"plugin_title_" . $plugin . "\"/>" .
		    "</primaryie></indexentry>\n";

	    }
	    print OUT "\t    </indexdiv>\n";

	    # switch back to normal mode
	    $mode = "normal";
	    print OUT $line;
	    next LINE;
	}
    }

}

close(IN);
close(OUT);

#############################################################################
#############################################################################
