#!/usr/bin/perl -w

#****c* Box/SquareBox
# FUNCTION
#   A box with the property that are sides are equal.
# ATTRIBUTES
#   SIDE_LENGTH -- the length of each side
# DERIVED FROM
#   Box
# SOURCE

package SquareBox;

use Box;
use vars ('@ISA');
@ISA = ("Box");

sub new {
    my $classname = shift;
    my $self      = $classname->SUPER::new(@_);
    $self->{SIDE}  = 1;
    return $self;
}

#*******


#****m* Box/SquareBox::side
# FUNCTION
#   Set or get the side length of the square box.
# SYNOPSIS
#   $boxref->side(100.25);
#   my $length = $boxref->side();
# RETURN VALUE
#   The volume of the box
# SOURCE

sub side {
    my $self = shift;
    if (@_) {
        my $length = shift;
        $self->{SIDE} = $length;
    }
    return  $self->{SIDE};
}

#*******

#****m* Box/SquareBox::volume
# FUNCTION
#   Compute the volume of a square box.
# SYNOPSIS
#   my $volume = $boxref->volume();
# RETURN VALUE
#   The volume of the box
# SOURCE

sub volume {
    my $self = { };
    return $self{SIDE} * $self{SIDE} * $self{SIDE} ;
}

#*****
1;

