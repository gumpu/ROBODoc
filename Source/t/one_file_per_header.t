#!perl

#------------------------------------------------------------------------------
use strict;
use warnings;
use ROBOTestFrame;
use Test::More 'no_plan';
use Test::File;

#****x* Basics/One file per header
# FUNCTION
#   Test --multidoc with --one_file_per_header for html output.  We
#   test this with one source file that contains three headers.  These
#   should result in three documentation files.  To make it nasty we
#   use some special header names.
#
# SOURCE

#    A dummy header to put into dummy source files.
my $source = <<'EOF';
/****f* Test/Test
 * NAME
 *   Test foo bar
 ******
 */

/****f* Test/foo, bar
 * NAME
 *   Test foo bar
 ******
 */

/****f* Test/aa::awaw
 * NAME
 *   Test foo bar
 ******
 */

EOF

{
    add_source( "test.c", $source );
    my ( $out, $err ) = runrobo(qw(--src Src --doc Doc --one_file_per_header --multidoc --html));
    # expected results:
    is( $out, '', 'No ouput' );
    is( $err, '', '... and no error' );
    # There are three headers, so there should be three documentation files.
    file_exists_ok( 'Doc/test_cTest2FTest.html',       'Documentation for Test/Test' );
    file_exists_ok( 'Doc/test_cTest2Ffoo.html',        'Documentation for Test/foo' );
    file_exists_ok( 'Doc/test_cTest2Faa3A3Aawaw.html', 'Documentation for Test/aa::awaw' );
    # And a style sheet.
    file_exists_ok( "Doc/robodoc.css", 'and a stylesheet' );
    clean();
}

#*******

