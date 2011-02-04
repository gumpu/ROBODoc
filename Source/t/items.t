#!perl
# vi: spell ff=unix
use strict;
use warnings;
use ROBOTestFrame;
use Test::More 'no_plan';

#****h* ROBODoc System Tests/Item Test
# FUNCTION
#   Tests the parsing generation of ROBODoc items.
#
#*****

#****x* Item Test/Sorting
# FUNCTION
#   Lets see if we can get items sorted according
#   to the order specified in a robodoc.rc file.
#
# SOURCE
#
{
    my $source = <<'EOF';
/****f* Test/test
 * FIRST
 *   Test 1
 * Second
 *   Test 2
 * THIRD
 *   Test 3
 * SOURCE
 */

  Test 4

 /*******/
EOF

    my $config = <<'EOF';
items:
    FIRST
    Second
    THIRD
    FOURTH
item order:
    SOURCE
    THIRD
EOF

    my $config_no_sort = <<'EOF';
items:
    FIRST
    Second
    THIRD
    FOURTH
EOF

    add_source( "test.c", $source );
    add_configuration( "robodoc.rc", $config );
    my ($out, $err) = runrobo( qw(
        --src Src
        --doc Doc --multidoc --ascii
        --rc Config/robodoc.rc
    ) );
    is( $out, '', 'no output' );
    is( $err, '', 'and no error' );
    my $documentation = IO::File->new( "<Doc/test_c.txt" );
    ok( $documentation, 'There is a doc file' );
    my @items = ();
    while ( my $line = <$documentation> ) {
        if ( $line =~ m/(SOURCE|THIRD|Second|FIRST)/ ) {
            push( @items, $1 );
        }
    }
    is( $items[ 0 ], 'SOURCE', 'First item it the source item' );
    is( $items[ 1 ], 'THIRD',  'Second item it the third item' );
    is( $items[ 2 ], 'FIRST',  'Third item it the first item' );
    $documentation->close();

    # Now the same but without sorting
    add_configuration( "robodoc.rc", $config_no_sort );
    my ($out2, $err2) = runrobo( qw(
        --src Src
        --doc Doc --multidoc --ascii
        --rc Config/robodoc.rc
    ) );
    is( $out2, '', 'no output' );
    is( $err2, '', 'and no error' );

    $documentation = IO::File->new( "<Doc/test_c.txt" );
    ok( $documentation, 'There is a doc file' );
    @items = ();
    while ( my $line = <$documentation> ) {
        if ( $line =~ m/(SOURCE|THIRD|Second|FIRST)/ ) {
            push( @items, $1 );
        }
    }
    is( $items[ 0 ], 'FIRST',  'First item it the first item' );
    is( $items[ 1 ], 'Second', 'Second item it the second item' );
    is( $items[ 2 ], 'THIRD',  'Third item it the third item' );
    is( $items[ 3 ], 'SOURCE', 'Fourth item it the fourth' );
    $documentation->close();

    clean();
}
#*******


