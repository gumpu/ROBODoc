#!/usr/bin/perl -w
# vi: ff=unix spell

#****h* ROBODoc System Tests/Encoding
# FUNCTION
#    Test decoding en encoding of non 7bit ASCII characters.
#*****

use strict;
use warnings;
use ROBOTestFrame;
use Test::More 'no_plan';
use Test::File;
use XML::Simple;
use Data::Dumper;

#
# This tests a header with item names that start with 
# an letter with an umlaut.
# ROBODoc currently has some problems with this.
#

{
    my $source = read_hexdump( 'TestData/iso8859-1_german_header_c.xxd' );
    my $config = read_hexdump( 'TestData/iso8859-1_german_robodoc_rc.xxd' );

    add_source( "test.c", $source, 'binary' );
    add_configuration( "test.rc", $config, 'binary' );
    mkdocdir();
    my ( $out, $err ) = runrobo(qw(
        --src Src
        --doc Doc/test
        --rc Config/test.rc
        --singledoc --test --nopre ));
    # expected results:
    is( $out, '', 'No ouput' );
    is( $err, '', '... and no error' );
    file_exists_ok( "Doc/test.xml", 'there should be documentation' );

    my $documentation = XMLin( 'Doc/test.xml' );
    my $header = $documentation->{'header'};
#    is ( $header->{'name'}, 'Test Foo Bar/Name With Spaces', 'Header name' );
    my $items = $header->{'item'};
    print Dumper( $items );
    my $body = $items->{'FUNCTION'}->{'item_body'};

    # TODO  currently a bug in robodoc.
    is( $body->{'pre'}, undef, 'No preformatted stuff in the body' );
    clean();
}


