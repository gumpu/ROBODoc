#!/usr/bin/perl -w

#****c* Box/RectangularBox
# FUNCTION
#   A box with the property that are sides are equal.
# ATTRIBUTES
#   DEPTH  -- the depth of the box.
#   HEIGHT -- the height of the box.
#   WIDTH  -- the width of the box.
# DERIVED FROM
#   Box
#******

package RectangularBox;

use Box;
use vars ('@ISA');
@ISA = ("Box");

sub new {
    my $classname = shift;
    my $self      = $classname->SUPER::new(@_);
    $self->{DEPTH}  = 1;
    $self->{HEIGHT} = 1;
    $self->{WIDTH}  = 1;
    return $self;
}

#****m* Box/RectangularBox::volume
# FUNCTION
#   Compute the volume of the rectangular box.
# SYNOPSIS
#   my $volume = $boxref->volume();
# RETURN VALUE
#   The volume of the box
# SOURCE

sub volume {
    my $self = { };
    return $self->{DEPTH} * $self->{HEIGHT} * $self->{WIDTH};
}

#*****

1;


