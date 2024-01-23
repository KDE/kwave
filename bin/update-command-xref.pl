#!/usr/bin/perl
#############################################################################
##    Kwave                - update-command-xref.pl
##                           -------------------
##    begin                : Sun Jan 04 2015
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

my @scanned_cmds;

sub scan_file
{
    my $file = shift;
    open(IN2, $file);
    while (<IN2>) {
        my $line = $_;

        chomp $line;
        $line =~ s/\s+$//;
        $line =~ s/^\s+|\s+$//g;

        if ($line =~ /^CASE_COMMAND\s*\(\"(.+)\"\s*\)/) {
            my $cmd = $1;
            push(@scanned_cmds, $cmd) if (! grep {$_ eq $cmd} @scanned_cmds);
        }
    }
    close(IN2);
}

sub process_dir
{
    my $path = shift;

    opendir(DIR, $path) or die "opendir $path: $!";
    my @files = grep { !/(^\.)/ } readdir(DIR);
    closedir(DIR);
    @files = map { $path . '/' . $_ } @files;

    for (@files) {
        if (-d $_) {
            process_dir($_);
        } else {
            my $file = $_;
            if ( $file =~ /\.(inl|cxx|cpp)$/ ) {
                scan_file($file);
            }
        }
    }
}

sub make_stub
{
    local $cmd = shift;
    local $lbl = $cmd;
    print OUT "\t\<\!-- \@COMMAND\@ " . $cmd . "(TODO) --\>\n";
    $lbl =~ s/[^\w]/_/g;
    print OUT "\t<sect2 id=\"cmd_sect_" . $lbl . "\">" .
              "<title id=\"cmd_title_" . $lbl . "\">" .
              "&no-i18n-cmd_" . $lbl . ";</title>\n";
    print OUT "\t<para>\n";
    print OUT "\t    TBD\n";
    print OUT "\t</para>\n";
    print OUT "\t</sect2>\n";
    print OUT "\n";
}

#
# recursively scan all files and subdirs for command names
#
process_dir($src_dir);
@scanned_cmds = sort { $a cmp $b } @scanned_cmds;
my @remaining_cmds = @scanned_cmds;

my $linenr = 0;
my $mode = "normal";
my $last_cmd = "";

LINE: while (<IN>) {
    my $line = $_;
    $linenr += 1;

    if ($mode eq "normal") {

        if ($line =~ /\<\!\-\-\ \@COMMAND_INDEX_START\@\ \-\-\>/) {
            print OUT $line;
            $mode = "index";
            next LINE;
        }

        if ($line =~ /\<\!\-\-\ \@COMMAND_ENTITIES_START\@\ \-\-\>/) {
            print OUT $line;
            $mode = "entities";
            next LINE;
        }

        if ($line =~ /\<\!\-\-\ \@COMMAND\@\ ([\w\:]+)\(([\w\,\:\[\]\.]*)\)\ \-\-\>/) {
            my $cmd = $1;
            my $params = $2;
            # print "### found command '" . $cmd . "' params: '" . $params . "'\n";
            if (not grep {$_ eq $cmd} @scanned_cmds) {
                print STDERR "WARNING: command '" . $cmd . "' is not supported / does not exist\n";
            } else {
                while (@remaining_cmds) {
                    my $next_cmd = shift @remaining_cmds;

                    # if command is next in list of remaining -> remove from list
                    # print "next_cmd=".$next_cmd.", cmd=".$cmd."\n";
                    if ($cmd eq $next_cmd) {
                        # print "(FOUND)\n";
                        # show a warning if there documentation is not finished
                        # for this command
                        if ($params eq "TODO") {
                            print STDERR "WARNING: command '" . $cmd . "': documentation is not finished (TODO)\n";
                        }
                        print OUT $line;
                        next LINE;
                    }

                    if ($cmd gt $next_cmd) {
                        print STDERR "WARNING: command '" . $next_cmd . "' is undocumented, adding stub\n";
                        make_stub $next_cmd;
                    } else {
                        last;
                    }
                }
            }
            print OUT $line;
            next LINE;
        }

        if ($line =~ /\<\!\-\-\ \@COMMAND_END_OF_LIST\@\ \-\-\>/) {
            # print "WARNING: some commands left\n";
            while (@remaining_cmds) {
                local $cmd = shift @remaining_cmds;
                print STDERR "WARNING: command '" . $cmd . "' is undocumented, adding stub\n";
                make_stub $cmd;
            }
        }

        print OUT $line;
    }

    if ($line =~ /\<\!\-\-\ \@COMMAND_ENTITIES_END\@\ \-\-\>/) {
        foreach (@scanned_cmds) {
            local $cmd = $_;
            local $lbl = $cmd;
            $lbl =~ s/[^\w]/_/g;
            print OUT "  \<\!ENTITY no-i18n-cmd_" . $lbl . " \"" . $cmd . "\"\>\n";
        }
        print OUT $line;
        $mode = "normal";
        next LINE;
    }

    if ($mode eq "index") {
        if ($line =~ /\<\!\-\-\ \@COMMAND_INDEX_END\@\ \-\-\>/) {

            # create a new command index
            my $first_char = "*";
            foreach (@scanned_cmds) {
                my $cmd = $_;
                my $first = substr($cmd, 0, 1);
                if (not ($first eq $first_char)) {
                    print OUT "\t    </indexdiv>\n" if (not ($first_char eq "*"));
                    print OUT "\t    <indexdiv><title>" . $first . "</title>\n";
                    $first_char = $first;
                }

                $cmd =~ s/[^\w]/_/g;
                print OUT "\t\t<indexentry><primaryie>" .
                    "<link linkend=\"cmd_sect_" . $cmd . "\" " .
                    "endterm=\"cmd_title_" . $cmd . "\"/>" .
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
