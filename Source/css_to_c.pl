#!/usr/bin/perl -w
#****h* Module/css_to_c.pl
# USAGE: 
#   ./css_to_c.pl 
#
# FUNCTION
#   Turns a .css file into a piece of C code.
#
# AUTHOR  
#   Frans Slothouber (FSL), <rfsber@xs4all.nl>  Gumpu Consulting
# CREATED:  
#   6-7-2006 12:45:01 W. Europe Standard Time
#     REVISION:  ---
#
#******************************************************************************
# $Id: css_to_c.pl,v 1.4 2006/07/27 19:12:18 gumpu Exp $

use strict;
use warnings;
use IO::File;


# First preprocess the CSS code.
my $robodoc_css_file_name = "robodoc_basic_style.css";
my $robodoc_css_file = IO::File->new("<$robodoc_css_file_name") or die;
my @css = <$robodoc_css_file>;
$robodoc_css_file->close();

# Remove \n
# Replace all % by %%
# Replace all " by \"
# wrap inside a " .. \n"
@css = map { chomp($_); s/%/%%/g;  s/"/\\"/g; "\"$_\\n\"\n" } @css;

# Split the whole file into several parts each with
# it's own  fprintf().
# This to avoid compiler warning about string that
# are too long.
my @split_css_code = ();
my $l = 0; # Length of the string
push( @split_css_code, "            fprintf( css_file,\n" );
foreach my $line ( @css )
{
    push( @split_css_code, $line );
    $l += length( $line );
    if ( $l > 2000 ) {
        $l = 0;
        push( @split_css_code, "                    );\n" );
        push( @split_css_code, "            fprintf( css_file,\n" );
    }
}
push( @split_css_code, "                    );\n" );


# Insert CSS code into  html_generator.c

# Read the original code.
my $html_generator_file = IO::File->new("<html_generator.c") or die;
my @code = <$html_generator_file>;
$html_generator_file->close();

# Create a backup.
my $html_generator_file_bak = IO::File->new(">html_generator_bak.c") or die;
print $html_generator_file_bak @code;
$html_generator_file_bak->close();

# Merge the C code with the CSS code.
my @new_code = ();
my $skip = 0;
foreach my $line ( @code ) {
    if ( $line =~ m/BEGIN\sBEGIN\s/ ) {
        $skip = 1;
        push( @new_code, $line );
        push( @new_code, @split_css_code );

    } elsif ( $line =~ m/END\sEND\s/ ) {
        push( @new_code, $line );
        $skip = 0;
    } else {
        if ( $skip ) {
            #nothing
        } else {
            push( @new_code, $line );
        }
    }
}

# Write the result.
$html_generator_file = IO::File->new(">html_generator.c") or die;

print $html_generator_file @new_code;

$html_generator_file->close();

