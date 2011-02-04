#!perl
# vi: spell ff=unix
#------------------------------------------------------------------------------
use strict;
use warnings;
use ROBOTestFrame;
use Test::More 'no_plan';
use Test::File;
use XML::Simple;

#****h* ROBODoc System Tests/Basics
# FUNCTION
#   Tests basic ROBODoc funcionallity.  These are all
#   'happy paths'.
#
#****

#****v* Basics/Dummy Headers
# FUNCTION
#   A dummy header to put into dummy source files.
# SOURCE
#
my $source = <<'EOF';
/****f* Test/test
 * NAME
 *   Test
 ******
 */
EOF
#*****

#****x* Basics/Option version
# FUNCTION
#   Test robodoc --version, this should report the current version.
# SOURCE
{
    my ( $out, $err ) = runrobo('--version');
    like( $out, qr/\d+\.\d+.\d+/, '--version should print version number' );
    is( $err, '', '... and no error' );
}
#****

#****x* Basics/Option help
# FUNCTION 
#   Test the option --help.
# SOURCE
{
    my ( $out, $err ) = runrobo('--help');
    like( $out, qr/ROBODoc Version/, '--help should print version number' );
    is( $err, '', '... and no error' );
}
#****


#****x* Basics/Option multidoc
# FUNCTION
#   Test --multidoc for html output format.
#   (Multidoc for other modes does not make much sense).
#   We create one source file with a simple header and create multidoc
# SOURCE
{
    add_source( "test.c", $source );
    my ( $out, $err ) = runrobo(qw(--src Src --doc Doc --multidoc --html));
    # expected results:
    is( $out, '', 'No ouput' );
    is( $err, '', '... and no error' );
    file_exists_ok( "Doc/test_c.html", 'there should be documentation' );
    file_exists_ok( "Doc/robodoc.css", 'and a stylesheet' );

    clean();
}
#****


# TODO FS
# test --multidoc for --troff
#

# TODO FS
# test --css
#

#****x* Basics/Singledoc Outputmode with Different Formats
# FUNCTION
#   Test singledoc mode  for all supported output formats 
#   and see if it creates output in this format.
#
# SOURCE

{
    add_source( "test.c", $source );

    my %output_modes = (
        '--html'  => 'html',
        '--rtf'   => 'rtf',
        '--test'  => 'xml', 
        '--latex' => 'tex',
        '--dbxml' => 'xml',
    );

    foreach my $mode ( keys %output_modes ) {
        my $file_extension = $output_modes{$mode};

        my @arguments = qw(--src Src --doc testdoc --singledoc);
        push( @arguments, $mode );

        my ( $out, $err ) = runrobo(@arguments);
        is( $out, '', 'No ouput' );
        is( $err, '', '... and no error' );
        file_exists_ok( "testdoc.$file_extension",
            'there should be documentation' );
        if ( $mode eq "--html" ) {
            file_exists_ok( "testdoc.css", 'and a stylesheet' );
        }

        unlink("testdoc.$file_extension") if -e "testdoc.$file_extension";
        unlink('testdoc.css')             if -e 'testdoc.css';

    }

    clean();
}

#****

#****v* Basics/test_source_2
# FUNCTION
#   A dummy header to put into dummy source files.
#   This one has headers at several levels.
# SOURCE
#
my $test_source_2 = <<'EOF';
/****f* Level_1/Level_2
 * NAME
 *   Test
 ******
 */

/****f* Level_2/_Level_3
 * NAME
 *   Test
 ******
 */

/****f* Level_3/_Level_4
 * NAME
 *   Test
 ******
 */

EOF
#*****


#****x* Basics/Option first section level
# FUNCTION
#   Test --first_section_level.
#
#   The level of the first section should be
#   at level 3 instead of 1.
#
# SOURCE
{
    add_source( "test.c", $test_source_2 );
    mkdocdir();
    my ( $out, $err ) = runrobo(
        qw(--src Src
           --doc Doc/test
           --singledoc
           --sections
           --test
           --toc
           --first_section_level 3));
    # expected results:
    is( $out, '', 'No ouput' );
    is( $err, '', '... and no error' );
    file_exists_ok( "Doc/test.xml", 'there should be documentation' );

    # Now test the level of the first header.
    my $documentation = XMLin( 'Doc/test.xml' );
    my $section = $documentation->{'section'};
    is ( $section->[0]->{'depth'}, '3', 'First section is at level 3' );
    my $subsection = $section->[0]->{'section'};
    is ( $subsection->{'depth'}, '4', 'First subsection is at level 4' );
    is ( $section->[1]->{'depth'}, '3', 'Second section is at level 3' );

    clean();
}
#****


1;

