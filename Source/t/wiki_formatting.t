#!perl
# vi: spell ff=unix
#****h* ROBODoc System Tests/Wiki Formatting
# FUNCTION
#    Tests that test the Wiki like formatting that ROBODoc supports.
#*****


#------------------------------------------------------------------------------
use strict;
use warnings;
use ROBOTestFrame;
use Test::More 'no_plan';
use Test::File;
use XML::Simple;
use Data::Dumper;

#****x* Wiki Formatting/Wiki Basics
# FUNCTION
#   Test a simple header: contains three lists, some paragraphs,
#   and some source.  All should be recognized.
# SOURCE

{
    my $source = <<'EOF';
/****f* Test/Test
 * NAME
 *
 *   Implements serializers for the following
 *   files:
 *   - DZB_ACG - SAP accounting file record.
 *   - DZB_RRP - regularoty reporting file record.
 *   - DZB_MVT - Exchange Position File Record.
 *
 *   A test
 *     
 *   Implements the following
 *   functions:
 *   - S99304_SERIALIZE_DZB_ACG
 *   - S99304_SERIALIZE_DZB_ACG_TBL
 *   - S99304_SERIALIZE_DZB_MVT
 *   and the functions:
 *   - S99304_SERIALIZE_DZB_MVT_TBL
 *   - S99304_SERIALIZE_DZB_RRP
 *   - S99304_SERIALIZE_DZB_RRP_TBL
 * SOURCE
 */
    test()

 /******/

EOF

    add_source( "test.c", $source );
    my ( $out, $err ) = runrobo(qw(--src Src --doc Doc --nopre --multidoc --test));
    # expected results:
    is( $out, '', 'No ouput' );
    is( $err, '', '... and no error' );

    my $documentation = XMLin( 'Doc/test_c.xml' );
    my $header = $documentation->{'header'};
    is ( $header->{'name'}, 'Test/Test', 'Header is named Test/Test' );
    my $items = $header->{'item'};
    ok ( exists( $items->{'NAME'} ),   'header has an item NAME' );
    ok ( exists( $items->{'SOURCE'} ), 'header has an item SOURCE' );
    my $body = $items->{'NAME'}->{'item_body'};

    # There are paragraphs.
    ok ( exists( $body->{'para'} ),    'item has paragraphs' );

    # There are three lists.
    is ( scalar( @{ $body->{'list'} } ), 3, 'item has three lists' );
    clean();
}

#******


