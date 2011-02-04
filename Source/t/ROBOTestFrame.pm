# vi: ff=unix spell
#****h* ROBODoc/ROBOTestFrame
# FUNCTION
#   A Perl module with a set of handy functions to create
#   test scripts.
#
#   These function are:
#   * runrobo
#   * add_source
#   * add_configuration
#   * clean
#   * mkdocdir
#   * is_latex_balanced
#   * read_hexdump
#
#*****

package ROBOTestFrame;
require Exporter;
  @ISA    = qw(Exporter);
  @EXPORT = qw(
    runrobo
    add_source
    add_configuration
    clean mkdocdir
    is_latex_balanced
    read_hexdump
    );  # symbols to export on request

use strict;
use warnings;
# TODO  Try to get this to work without IPC::Run
use IPC::Run 'run';
use File::Path;
use File::Basename;
use IO::File;

#****f* ROBOTestFrame/robo_win, robo_unix
# FUNCTION
#   Location of the ROBODoc executable.
# SOURCE
#
my $robo_win = "../robodoc.exe";
my $robo_unix = "../robodoc";
#****

#****f* ROBOTestFrame/source_directory
# FUNCTION
#   Name of the source directory used to test ROBODoc.
# SOURCE
#
my $source_directory  = "Src";
#*****

#****f* ROBOTestFrame/documentation_directory
# FUNCTION
#   Name of the documentation directory used to test ROBODoc.
# SOURCE
my $documentation_directory  = "Doc";
#*****

#****f* ROBOTestFrame/configuration_directory
# FUNCTION
#   Name of the documentation directory used to test ROBODoc.
# SOURCE
my $configuration_directory = "Config";
#*****

#****f* ROBOTestFrame/runrobo
# FUNCTION
#   Run robodoc with the given set of
#   arguments and capture all output to
#   stdout en stderr.
# INPUTS
#   A list of options for robodoc.exr
# RETURNS
#   stdout and stderr.
# SOURCE
#
sub runrobo
{
    my $robo = '';
    if ( $^O eq 'linux' ) {
	$robo = $robo_unix;
    } else {
	$robo = $robo_win;
    }
    run( [ $robo, @_ ], \my( $in, $out, $err ) );
    return ($out, $err);
}
#*****


#****f* ROBOTestFrame/add_configuration
# FUNCTION
#   Add a configuration file somewhere in Config/
# INPUTS
#   - filepath -- path to a file.
#   - configuration -- the content for this file
#   - binary -- write the raw bytes.  [optional]
# SOURCE
#
sub add_configuration
{
    my $filepath = shift;
    my $configuration   = shift;
    my $binary     = shift;
    add_file( $configuration_directory, $filepath, $configuration, $binary )
}

#*****

#****f* ROBOTestFrame/add_source
# FUNCTION
#   Add a single source file somewhere in Src/
# INPUTS
#   - filepath -- path to a file.
#   - source_code -- the source code to go into this file
#   - binary -- write the raw bytes. [optional]
# SOURCE
#
sub add_source
{
    my $filepath = shift;
    my $source_code   = shift;
    my $binary     = shift;

    add_file( $source_directory, $filepath, $source_code, $binary )
}

#*****

#****f* ROBOTestFrame/add_file
# FUNCTION
#   Add a single file somewhere in base_path.
# INPUTS
#   - base_path -- base path
#   - filepath -- relative path to a file.
#   - content  -- the content to go into this file
#   - binary -- write the raw bytes. [optional]
# SOURCE
#

sub add_file
{
    my $base_path = shift;
    my $filepath  = shift;
    my $content   = shift;
    my $binary    = shift;

    my $path = $base_path . dirname( $filepath );

    $path =~ s/\.$//;  # Fix for Perl 5.8.0 under Linux.

    if ( ! -e "$path" ) {
        mkpath $path or die "can't create $path";
    }

    my $full_filepath = "$base_path/$filepath";
    my $file = IO::File->new(">$full_filepath") or 
         die "Can't open $full_filepath";
    if ( $binary and ( $binary eq 'binary' ) ) {
        binmode( $file );
    }
    print $file $content;
    $file->close();
}
#*****


#****f* ROBOTestFrame/clean
# FUNCTION
#    Clean source and documentation directories.
# SOURCE
#
sub clean
{
    if ( -e $source_directory ) {
        rmtree( $source_directory ) or die;
    }
    if ( -e $documentation_directory ) {
        rmtree( $documentation_directory ) or die;
    }
    if ( -e $configuration_directory ) {
        rmtree( $configuration_directory ) or die;
    }
}

#*****

#****f* ROBOTestFrame/mkdocdir
# FUNCTION
#   Create a empty documentation directory.
#   This is handy for tests that use --singledoc.
# SOURCE
sub mkdocdir
{
    if ( ! -e $documentation_directory ) {
        mkpath( $documentation_directory );
    }
}

#****

#****f* ROBOTestFrame/is_latex_balanced
# FUNCTION
#   Test the balance of a latex file.
#   A latex file is balanced if every
#     /begin{xxxx}
#   is ended with a
#     /end{xxx}
# INPUTS
#   * path - path to a latex file.
# RETURNS
#   * 0 -- file is not balanced
#   * 1 -- file is balanced
# SOURCE

sub is_latex_balanced {
    my $path = shift;
    my @stack;
    local( $/ ) ;
    my $file = IO::File->new("<$path") or die "$path : $!";
    my $string = <$file>;
    $file->close();

    while ( $string =~ m/(begin|end)\{([^}]+)\}/g ) {
        my $b_e  = $1;  # begin or end
        my $kind = $2;  # document, or equation, or ...
        if ( $b_e eq "begin" ) {
            push( @stack, $kind );
        } else {
            if ( pop( @stack ) eq $kind ) {
                # OK.  begin and end matches.
            } else {
                # Not OK!  
                #   begin{ something }
                # followed by 
                #   end{ something else }
                return 0;  # Failure.
            }

        }
    }
    if ( scalar( @stack ) ) {
        # there are items left!
        return 0; # Not OK.
    }
    return 1;  # OK!
}
#******


#****f* ROBOTestFrame/read_hexdump
# FUNCTION
#   Reads a hexdump made with xxd (part of vim http://www.vim.org/) 
#   This makes it possible to add files with all kinds of
#   different formats and characters.
#
#   Storing it in hexdump format makes sure that these files are
#   not changed when they are checked into cvs or unzipped.
#
#   This makes is possible to test cr/lf problems and internationalization
#   issues.
#
# INPUTS
#   * path - path to a hexdump file.
# RETURNS
#   The decoded content of the file as a single string.
# SOURCE

sub read_hexdump {
    my $path = shift;
    my $file = IO::File->new("<$path") or die "$path : $!";

    my $string = '';
    my @all_bytes = ();
    while ( my $line = <$file> ) {
        $line =~ s/^\S+:\s//; # remove address
        $line =~ s/\s\s+.*$//; # remove ascii
        $line =~ s/(\S\S)(\S\S)/$1 $2/g;
        # Now only the words are left.
        my @data = split( /\s/, $line );
        my @bytes = map { chr hex } @data;
        push( @all_bytes, @bytes );
    }
    # TODO try a join() here.
    foreach my $c ( @all_bytes ) {
        $string .= $c;
    }

    $file->close();
    return $string;
}

#******

1;
