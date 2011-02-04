use strict;
use warnings;
use ROBOTestFrame;
use Test::More 'no_plan';
use Test::File;

#****h* ROBODoc System Tests/LaTeX Generator
# FUNCTION
#   Test ROBODoc LaTeX generator.
#
#****


#****v* LaTeX Generator/dummy_header_1
# FUNCTION
#   A dummy header to put into dummy source files.
# SOURCE
#
    my $dummy_header_1 = <<'EOF';
C     ****f* Lib/Func
C     NAME
C       Func -- useless
C       Computes the value:
C        |latex \begin{equation}
C        |latex x = 0
C        |latex \end{equation}
C        app
C
C     SYNOPSIS
C       Func = Func (n)
C       Computes the value:
C        |latex \begin{equation}
C        |latex x = 0
C        |latex \end{equation}
C     BUGS
C       Generates screwy TeX
C     ***
      real function Func(n)
        Func = 0
      end function Func

EOF

#****


#****v* LaTeX Generator/dummy_header_2
# FUNCTION
#   A dummy header to put into dummy source files.
# SOURCE
#
    my $dummy_header_2 = <<'EOF';
C     ****f* Lib/Func
C     NAME
C       Func -- useless
C     SYNOPSIS
C       Example:
C         Foo foo foo
C         foo
C
C       Test paragraph.
C       Do da diddi do da dom dom.
C
C     BUGS
C       A list test:
C       * item 1
C       * item 2
C       * item 3
C
C     ***
      real function Func(n)
        Func = 0
      end function Func

EOF

#****


#****x* LaTeX Generator/latex is balanced
# FUNCTION
#   This function tests whether a generated latex file is balanced
#   or not.  That is every 
#     /begin{xxx} 
#   should end with a 
#     /end{xxx}
#   at the same level.
#
#   This is tested with several headers and in different modes.
#
# SOURCE
{
    my @sources = ( \$dummy_header_1, \$dummy_header_2 );

    foreach my $source_ref ( @sources ) {
        foreach my $mode_1 qw( --sections --toc --index ) {
            foreach my $mode_2 qw( --nopre --altlatex ) {
                mkdocdir();
                add_source( "test.c", $$source_ref );
                my ( $out, $err ) = runrobo(
                    qw(--src Src
                    --doc Doc/test
                    --singledoc
                    --latex
                    ), $mode_1, $mode_2 );
                # expected results:
                is( $out, '', 'No ouput' );
                is( $err, '', '... and no error' );
                file_exists_ok( "Doc/test.tex", 'there should be documentation' );
                is( is_latex_balanced( "Doc/test.tex" ), 1, 'latex is balanced' );
                clean();
            }
        }
    }
}
#****

