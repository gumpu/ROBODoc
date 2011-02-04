#!perl
# vi: spell ff=unix
use strict;
use warnings;
use ROBOTestFrame;
use Test::More 'no_plan';
use XML::Simple;

#****h* ROBODoc System Tests/Header Test
# FUNCTION
#   Tests the parsing of ROBODoc headers.
#*****

#****x* Header Test/Happy Path
# FUNCTION
#   Happy path, a simple plain header.  This definitely should work
# SOURCE
#
{
    my $source = <<'EOF';
/****f* Test/test
 * NAME
 *   Test
 * FUNCTION
 *   Test2
 ******
 */
EOF

    add_source( "test.c", $source );
    my ($out, $err) = runrobo( qw(--src Src --doc Doc --multidoc --test) );
    is( $out, '', 'no output' );
    is( $err, '', 'and no error' );
    clean();
}
#*******


#****x* Header Test/Names with Spaces
# FUNCTION
#   Try a header name with spaces and some '*' at the end.  The '*' should be
#   ignored.
# SOURCE
{
    my $source = <<'EOF';
/****f* Test Foo Bar/Name With Spaces ****
 * NAME
 *   Test
 * FUNCTION
 *   Test2
 ******
 */
EOF

    add_source( "test.c", $source );
    my ($out, $err) = runrobo( qw(--src Src --doc Doc --multidoc --test) );
    is( $out, '', 'no output' );
    is( $err, '', 'and no error' );
    my $documentation = XMLin( 'Doc/test_c.xml' );
    my $header = $documentation->{'header'};
    is ( $header->{'name'}, 'Test Foo Bar/Name With Spaces', 'Header name' );

    clean();
}

#*****

#****x* Header Test/Multiple Names with Spaces
# FUNCTION
#   Try a header with names that contain spaces.
#   All of these should be accepted.
# SOURCE
{
    my $source = <<'EOF';
/****f* Test Foo Bar/Name With Spaces, And Anotherone,
 *                   And One More, More
 * NAME
 *   Test
 * FUNCTION
 *   Test2
 ******
 */
EOF

    add_source( "test.c", $source );
    my ($out, $err) = runrobo( qw(--src Src --doc Doc --multidoc --test) );
    is( $out, '', 'no output' );
    is( $err, '', 'and no error' );
    my $documentation = XMLin( 'Doc/test_c.xml' );
    my $header = $documentation->{'header'};
    is ( $header->{'name'}, 'Test Foo Bar/Name With Spaces', 'Header name' );
    clean();
}

#*****


#****x* Header Test/Circular header
# FUNCTION
#   It is possible to create loops in the header structure.  ROBODoc should
#   complain about this.
#   We test this by creating a loop.
# SOURCE
#
{
    my $source = <<'EOF';

/****f* Xxx/Yyy
* NAME
*   Bar, the last header points to this header.
****/

/****f* Yyy/Zzz
 * NAME
 *   Foo, dummy
 *****/

/****f* Yyy/Zzz5
 * NAME
 *   Foo, dummy
 *****/

****f* Yyy/Zzz4
 * NAME
 *   Foo, dummy
 *****/

/****f* Zzz/Xxx
 * NAME
 *   This header points back to the first header.
 *****/

EOF

    add_source( "test.c", $source );
    my ($out, $err) = runrobo( qw(
        --src Src
        --doc Doc
        --multidoc
        --html
        --toc
        --sections
        ) );
    # ROBODoc should complain about circular 
    # headers.
    isnt( $err, '', 'there should be an error' );
    clean();
}

#*****

