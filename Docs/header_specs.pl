#!/usr/bin/perl -w

use strict;
use warnings;
use IO::File;


#------------------------------------------
#
my @header_specs = (
    { language => [ 'C' ],
      begin => '/****',
      mid   => '*',
      end_1 => '****',
      end_2 => '/****',
      rem_begin => '/*',
      rem_end   => '*/' 
    },

    { language => [ 'Modula-2' ],
      begin => '(****',
      mid   => '*',
      end_1 => '****',
      end_2 => '(****',
      rem_begin => '(*',
      rem_end   => '*)' 
    },

    { language => [ 'Pascal' ],
      begin => '{****',
      mid   => '*',
      end_1 => '****',
      end_2 => '{****',
      rem_begin => '{*',
      rem_end   => '*}' 
    },

    { language => [ 'C++' ],
      begin => '//****',
      mid   => '//',
      end_2 => '//****',
    },

    { language => [ 'Tcl', 'Perl' ],
      begin => '#****',
      mid   => '#',
      end_2 => '#****',
    },

    { language => [ 'LaTeX/TeX', 'Postscript' ],
      begin => '%****',
      mid   => '%',
      end_2 => '%****',
    },

    { language => [ 'Occam' ],
      begin => '__****',
      mid   => '__',
      end_2 => '__****',
    },

    { language => [ 'Fortran 90' ],
      begin => '__****',
      mid   => '__',
      end_2 => '__****',
    },

);

#--------------------------------------------------------------------
#


sub make_robodoc_rc_file {
    my $spec = shift;
    my $rc_file = "";

    $rc_file .= "header markers:\n";
    $rc_file .= "  $spec->{begin}\n";
    $rc_file .= "remark markers:\n";
    $rc_file .= "  $spec->{mid}\n";
    $rc_file .= "end markers:\n";
    if ( exists( $spec->{end_1} ) ) {
        $rc_file .= "  $spec->{end_1}\n";
    }
    if ( exists( $spec->{end_2} ) ) {
        $rc_file .= "  $spec->{end_2}\n";
    }

    if ( exists( $spec->{rem_end} ) ) {
        $rc_file .= "remark begin markers:\n";
        $rc_file .= "  $spec->{rem_begin}\n";
        $rc_file .= "remark end markers:\n";
        $rc_file .= "  $spec->{rem_end}\n";
    }

    return $rc_file;
}


#
# given a header spec this returns an example header 
# that does not contain any source items.
#
sub make_example_header_simple {
    my $spec = shift;
    my $header = "";

    $header = $spec->{begin} . "f* ModuleName/Foo\n";
    $header .= $spec->{mid} . " FUNCTION\n";
    $header .= $spec->{mid} . "   Foo computes the foo factor\n";
    $header .= $spec->{mid} . "   using a fudge factor.\n";
    if ( exists( $spec->{end_1} ) ) {
        $header .= $spec->{end_1} . "\n";
        $header .= $spec->{rem_end} . "\n";
    } else {
        $header .= $spec->{end_2} . "\n";
    }
}


sub make_example_header {
    my $spec = shift;
    my $header = "";

    $header = $spec->{begin} . "f* ModuleName/Foo\n";
    $header .= $spec->{mid} . " FUNCTION\n";
    $header .= $spec->{mid} . "   Foo computes the foo factor\n";
    $header .= $spec->{mid} . "   using a fudge factor.\n";
    $header .= $spec->{mid} . " SYNOPSIS\n";
    if ( exists( $spec->{rem_end} ) ) {
        $header .= $spec->{rem_end} . "\n";
    } else {

    }
    $header .= "int Foo( int fudge )\n";
    if ( exists( $spec->{rem_begin} ) ) {
        $header .= $spec->{rem_begin} . "\n";
    } else {

    }
    $header .= $spec->{mid} . " INPUTS\n";
    $header .= $spec->{mid} . "   fudge -- the fudge factor\n";
    $header .= $spec->{mid} . " SOURCE\n";
    if ( exists( $spec->{rem_end} ) ) {
        $header .= $spec->{rem_end} . "\n";
    } else {

    }

    $header .= "\n more source code..\n\n";

    if ( exists( $spec->{rem_end} ) ) {
        $header .= $spec->{end_2};
        $header .= $spec->{rem_end} . "\n";
    } else {
        $header .= $spec->{end_2} . "\n";
    }

    return $header;
}




sub make_examples_for_manual {
    my $filename = "header_examples.xml";
    my $file = IO::File->new(">$filename") or die "$filename : $!";
    foreach my $spec ( @header_specs ) {
        foreach my $language ( @{$spec->{language}} ) {
            print $file "<section>\n";
            print $file "<title>$language</title>\n";
            print $file "\n\n";

            print $file "<example>\n";
            print $file "<title>A simple header without any source items in $language.</title>\n";
            print $file "<programlisting>\n";
            print $file make_example_header_simple( $spec );
            print $file "</programlisting>\n";
            print $file "</example>\n";
            print $file "\n\n";

            print $file "<example>\n";
            print $file "<title>A header with source items in $language.</title>\n";
            print $file "<programlisting>\n";
            print $file make_example_header( $spec );
            print $file "</programlisting>\n";
            print $file "</example>\n";
            print $file "\n\n";

            print $file "<example>\n";
            print $file "<title>The robodoc.rc file required for $language if it were not supported by default.</title>\n";
            print $file "<programlisting>\n";
            print $file make_robodoc_rc_file( $spec );
            print $file "</programlisting>\n";
            print $file "</example>\n";

            print $file "</section>\n";
        }
    }

    $file->close();
}



make_examples_for_manual;

