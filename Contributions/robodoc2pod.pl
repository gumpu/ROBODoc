#!/usr/bin/perl
###################################################
# robodoc 2 pod converter
###################################################
#****h* robodoc2pod
# NAME
# Robodoc 2 Pod
#
# FUNCTION
# Generate POD documentation from ROBODoc to allow
# use of perldoc with your Robodoc'ed code.
#
# HISTORY
# * V 0.2.1 of 06/03/14		corrected the regexps
# * V 0.2.0 of 06/03/13		rewritten with intermediate representation
# * V 0.1.0 of 06/03/10		first version
#
# BUGS
# nothing known right now.
#
# TODO
# * refactor cleanly
# * manage locales
# * manage nested lists
# * indent EXAMPLE with 
# 
# LICENSE
# This program is free software; you can redistribute it and/or modify it under the same terms as Perl itself. 
# AUTHOR
# Emmanuel Florac ( wazoox @ free . fr )
# COPYRIGHT
# (c) 2006 Intellique (www.intellique.com)
#***
# always use strict et warnings.
use strict;
use warnings;

use Data::Dumper;

#########################
# functions
#########################

sub usage {
    return "usage  : $0 <source file> [ >> <pod file> ]";
}

#########################
# main
#########################

# must provide a file name to work with
my $file = shift or die usage();
open my $fh, $file or die "can't open file : $file";

# robodoc start and end tags (marks robodoc blocks)
my $rbd_starttag = qr(^\*\*\*\*[\w\*]\*);
my $rbdheadtype  = qr(^\*\*\*\*([\w\*])\*);
my $rbd_endtag   = qr(^\*\*\*);

# robodoc general tags
my @rbdtags = (
    'NAME',          'COPYRIGHT',      'SYNOPSIS',    'USAGE',
    'FUNCTION',      'DESCRIPTION',    'PURPOSE',     'AUTHOR',
    'CREATION DATE', 'MODIFICATION',   'HISTORY',     'INPUTS',
    'ARGUMENTS',     'OPTIONS',        'PARAMETERS',  'SWITCHES',
    'OUTPUT',        'SIDE EFFECTS',   'RESULT',      'RETURN VALUE',
    'EXAMPLE',       'NOTES',          'DIAGNOSTICS', 'WARNINGS',
    'ERRORS',        'BUGS',           'TODO',        'IDEAS',
    'PORTABILITY',   'SEE ALSO',       'METHODS',     'NEW METHODS',
    'ATTRIBUTES',    'NEW ATTRIBUTES', 'TAGS',        'COMMANDS',
    'DERIVED FROM',  'DERIVED BY',     'USES',        'CHILDREN',
    'USED BY',       'PARENTS',        'SOURCE',	   'LICENSE',
);

my %rbdheaders = (
    c   => 'Class',
    d   => 'Constant',
    f   => 'Fonction',
    h   => 'Module',
    m   => 'Méthod',
    s   => 'Structure',
    t   => 'Type',
    u   => 'Unit Test',
    v   => 'Variable',
    '*' => '',
);

# to check for headers tags
my $tagmatch = join '|', @rbdtags;
$tagmatch = qr(^($tagmatch));

# to store the robodoc
my @robodoc;

# flag and titles
my $inrobodoc  = 0;
my $rbdtagname = '';

# read the file
while (<$fh>) {

    # remove leading # if any
    s/^\s*# *//;
    chomp;

    $inrobodoc = 0 if m/$rbd_endtag/;

    if ($inrobodoc) {
        push @{ $robodoc[$#robodoc]{$rbdtagname} }, $_;
    }

    if (m/$rbd_starttag/) {
        $inrobodoc = 1;
        my ($headertype) = (m/$rbdheadtype/);
        ($rbdtagname) = (m/$rbd_starttag(.*)/);
        chomp $rbdtagname;
        if ($rbdtagname) {
            $rbdtagname = $rbdheaders{$headertype} . $rbdtagname;
            push @robodoc, { $rbdtagname => [] };
        }
    }
}

close $fh;

# now convert robodoc to pod
my @pod;
my $items   = 0;
my $podhead = 1;

foreach (@robodoc) {
    my ( $k, $v ) = each %$_;
    my $currhead = $podhead;
    push @pod, '', "=head$currhead $k", '';
    $currhead++;

    foreach my $line (@$v) {
		# insert head if this is a managed tag
        if ( $line =~ m/$tagmatch/ ) {
            push @pod, ( '', "=head$currhead $line", '' );
		# insert bulleted lists
        } elsif ( my ($elem) = ( $line =~ m/^\*\s+(.*)/ ) ) {
            if ( $items == 0 ) {
                $items++;
                push @pod, "=over";
            }
            push @pod, ( '', '=item *', '', $elem );
		# end bulleted list
        } elsif ( $items > 0 ) {
            $items = 0;
            push @pod, ('', '=back', '');
            push @pod, $line;
		# raw insert
        } else {
            push @pod, $line;
        }
    }
}

print join( "\n", @pod ) . "\n";

