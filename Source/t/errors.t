#!perl

#****h* ROBODoc System Tests/Handling Errors
# FUNCTION
#    Test wether calling ROBODoc with wrong options or input leads
#    to the correct error messages.
#*****

use strict;
use warnings;
use ROBOTestFrame;
use Test::More 'no_plan';

# A dummy header to put into dummy source files.
#
my $source = <<'EOF';
/****f* Test/test
 * NAME
 *   Test
 ******
 */
EOF

#****x* Handling Errors/Error on Non-existing Option
# FUNCTION
#   ROBODoc should complain about non-existing options.
# SOURCE
#
{
    my ($out, $err) = runrobo( '--foobar' );
    like($out, qr/Usage/, 'Unknown option should print usage' );
    like($err, qr/Invalid/, 'and an error message'   );
}
#*****


#****x* Handling Errors/Error on Mutual Excluding Options
# FUNCTION
#   ROBODoc should complain about options that can not be used
#   together.
# SOURCE
#
{
    add_source( "test.c", $source );
    my ($out, $err) = runrobo( qw(--src Src --doc Doc --multidoc --singledoc --html) );
    like($out, qr/Usage/, 'Mutual excluding options should print usage' );
    print $err;
    like($err, qr/together/, 'and an error message'   );
    clean();
}
#*****

#****x* Handling Errors/Error on Mutual Excluding Options II
# FUNCTION
#   ROBODoc should complain about options that can not be used
#   together.
# SOURCE
#
{
    add_source( "test.c", $source );
    my ($out, $err) = runrobo( qw(--src Src --doc Doc --multidoc --test --dbxml --html) );
    like($out, qr/Usage/, 'Mutual excluding options should print usage' );
    print $err;
    like($err, qr/together/, 'and an error message'   );
    clean();
}
#*****

#****x* Handling Errors/Error on Duplicate Options
# FUNCTION
#   ROBODoc should complain about options that are specifed more
#   than once.
# SOURCE
#
{
    add_source( "test.c", $source );
    my ($out, $err) = runrobo( qw( --src Src --doc Doc --multidoc --test --test --test) );
    print $out;
    like($out, qr/Usage/, 'Duplicate options should print usage' );
    print $err;
    like($err, qr/than\sonce/, 'and an error message'   );
    clean();
}
#*****

#****x* Handling Errors/Error on Non-existing rc file
# FUNCTION
#   When given a non-existing .rc file, ROBODc should
#   at least report the name of the .rc file.
# SOURCE
{
    my ($out, $err) = runrobo( '--rc foobar.rc' );
    like($err, qr/foobar/, 'should give an error message about foorbar.rc' );
}
#*****

#****x* Handling Errors/Impossible output file
# FUNCTION
#   When given a impossible output filename ROBODoc should
#   at least report the filename.
# SOURCE
{
    add_source( "test.c", $source );
    my ($out, $err) = runrobo( qw(--src Src --doc Foo/Bar/document --singledoc --html) );
    like($err, qr/document/, 'should give an error message about document' );
}

#*****

#****x* Handling Errors/Non-existing css file
# FUNCTION
#   When given  a impossible css filename ROBODoc should
#   at least report the filename.
# SOURCE
{
    add_source( "test.c", $source );
    my ($out, $err) = runrobo( qw(--src Src --doc Doc --multidoc --html --css Foo/Bar/cascade.css) );
    like($err, qr/cascade/, 'should give an error message about css file' );
}

#*****



#****x* Handling Errors/Header without end marker
# FUNCTION
#   Test ROBODoc's response on a header without an
#   end marker. ROBODoc should detect this.
# SOURCE
#

{
    my $source = <<'EOF';
/****f* Test/test
 * NAME
 *   Test
 * FUNCTION
 *   Test2
 * SOURCE
 */

some source

and no end of the header  ....

EOF

    add_source( "test.c", $source );
    my ($out, $err) = runrobo( qw(--src Src --doc Doc --multidoc --test) );
    is( $out, '', 'no output' );
    like( $err, qr/end\smarker/, 'error about end marker' );
    clean();
}

#****

#****x* Handling Errors/Headers with duplicate names
# FUNCTION
#   Test ROBODoc's response to a file with two headers that
#   have the same name. This should be reported as an error.
# SOURCE

{
    my $source = <<'EOF';
/****f* Test/test
 * NAME
 *   Test
 * FUNCTION
 *   Test2
 *****
 */

/****f* Test/test
 * NAME
 *   Test
 * FUNCTION
 *   Test2
 *****
 */

EOF

    add_source( "test.c", $source );
    my ($out, $err) = runrobo( qw(--src Src --doc Doc --multidoc --test) );
    is( $out, '', 'no output' );
    like( $err, qr/already\sexists/, 'error duplicate header' );
    clean();
}

#*****

{
    my $source = <<'EOF';
/****f* Test/test
 * FOO
 *   test
 ******/
EOF

    my $rc_file = <<'EOF';
items:
    FOO
    BAR
    SNAFU
item order:
    FOO
    BARRRRRR
    SNAFU
EOF

    add_configuration( "robodoc.rc", $rc_file );
    add_source( "test.c", $source );
    mkdocdir();

    my ($out, $err) = runrobo( qw(
        --src Src 
        --doc Doc 
        --multidoc 
        --rc Config/robodoc.rc --test) );
    is( $out, '', 'no output' );
    like( $err, qr/item/, 'An unknown item is reported' );

    clean();
}
