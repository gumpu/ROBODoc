#/usr/bin/perl -w
#vi: spell ff=unix
use strict;
use warnings;
use ROBOTestFrame;
use Test::More 'no_plan';
use Test::File;


#****h* ROBODoc System Tests/Pipes
# FUNCTION
#   Test the usage of pipes.
#
#****


my $pipe_source = <<'EOF';
/****f* Test/Test
 * NAME
 *   Only "pipes" which match selected output style are picked up.
 *   |html <CENTER>This will be included in <B>HTML</B> output.</CENTER>
 *
 *   |latex \centerline{This will be included in \LaTeX output}
 *
 *   Space is mandatory following the pipe marker. The following is not a
 *   valid pipe marker:
 *   |html<B>Moi!</B>
 *
 *   You should see an equation on the following line:
 *   |html y = x^2 (sorry, plain HTML is not very powerful)
 *   |latex \centerline{$y = x^2$}
 *
 *   How does this look like?
 *
 *   Here comes a multi-line equation array:
 *    |latex \begin{eqnarray}
 *    |latex \frac{\partial u}{\partial \tau} & = & D_u {\nabla}^2 u +
 *    |latex \frac{1}{\epsilon}
 *    |latex \left ( \hat{u}-{\hat{u}}^2-f\, {v} \, \frac{\hat{u}-q}{\hat{u}+q}
 *    |latex \right ) \; ,  \label{diffspot:1} \\
 *    |latex \frac{\partial v}{\partial \tau} & = & \hat{u}-v \; ,
 *    |latex \label{diffspot:2} \\
 *    |latex \frac{\partial r}{\partial \tau} & = & D_r {\nabla}^2 r \; .
 *    |latex \label{diffspAot:3}
 *    |latex \end{eqnarray}
 *    |html <I>TODO: write this in html</I>
 *   Another paragraph
 * SOURCE
 */
    test()

 /******/

EOF


#****x* Pipes/latex pipes
# FUNCTION
#   Test the use of latex pipes preformatted more.
# |latex $2n < 8$
# SOURCE
{
    add_source( "test.c", $pipe_source );
    mkdocdir();
    my ( $out, $err ) = runrobo(
        qw(--src Src 
           --doc Doc/test
           --nopre
           --singledoc
           --sections
           --latex
           ));
    # expected results:
    is( $out, '', 'No ouput' );
    is( $err, '', '... and no error' );
    file_exists_ok( "Doc/test.tex", 'there should be documentation' );
    # TODO test content
    clean();
}
#****



