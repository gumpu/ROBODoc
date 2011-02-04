#!perl -w
# vim: spell ff=unix
#****h* ROBODoc System Tests/ROBODoc Configuration File
# FUNCTION
#    Test stuff that can be specified in the robodoc.rc file.
#*****

use strict;
use warnings;
use ROBOTestFrame;
use Test::More 'no_plan';
use XML::Simple;
use Data::Dumper;

#****x* ROBODoc Configuration File/Custom Header Markers
# FUNCTION
#   Test whether custum header markers can be specified and
#   work.
# SOURCE
#

{
    my $source = <<'EOF';
/ ****f* Test/test
/ NAME
/   Test
/ FUNCTION
/   Test1
/ SOURCE
/ ****

/ ****F* Foo/foo
/ NAME
/   Foo
/ FUNCTION
/   Test2
/ SOURCE
/ ****

EOF

    my $rc_file = <<'EOF';
header markers:
  / ****
remark markers:
  /
end markers:
  / ****
headertypes:
  f functions 1
  F Foos      2
EOF

    add_source( "test.c", $source );
    add_source( "robodoc.rc", $rc_file );
    my ($out, $err) = runrobo( qw(--src Src --doc Doc --multidoc 
        --test --rc Src/robodoc.rc ) );
    is( $out, '', 'no output' );
    is( $err, '', 'no error' );
    my $documentation = XMLin( 'Doc/test_c.xml' );
    my $header = $documentation->{'header'};
    # Now look for the headers.
    isnt ( $header->{'Foo/foo'}, undef, 'There is a header named Foo/foo' );
    isnt ( $header->{'Test/test'}, undef, 'There is a header named Test/foo' );
    clean();
}

#****

#****x* ROBODoc Configuration File/line-endings.
# FUNCTION
#   ROBODoc should not care about the kind of line-endings that
#   are used.  Either cr/lf or cr, or even lf should work without
#   any problem.   We test this with two .rc files that have
#   different kind of line-endings.
# SOURCE
#

{
    my $source = <<'EOF';
/****f* Test/test
 * FOO
 *   test
 ******/
EOF
    # A rc file with 'unix' line-endings.
    my $config = read_hexdump( 'TestData/robodoc_unix_rc.xxd' );
    add_configuration( "test.rc", $config, 'binary' );
    add_source( "test.rc", $source );
    mkdocdir();
    my ( $out, $err ) = runrobo(qw(
        --src Src
        --doc Doc/test
        --rc Config/test.rc
        --singledoc --test --nopre ));
    # expected results:
    is( $out, '', 'No ouput' );
    is( $err, '', '... and no error' );
    clean();

    # A rc file with 'windows' line-endings.
    $config = read_hexdump( 'TestData/robodoc_windows_rc.xxd' );
    add_configuration( "test.rc", $config, 'binary' );
    add_source( "test.rc", $source );
    mkdocdir();
    ( $out, $err ) = runrobo(qw(
        --src Src
        --doc Doc/test
        --rc Config/test.rc
        --singledoc --test --nopre ));
    # expected results:
    is( $out, '', 'No ouput' );
    is( $err, '', '... and no error' );
    clean();
}

#******

