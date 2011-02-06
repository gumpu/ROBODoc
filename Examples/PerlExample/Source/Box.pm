#!/usr/bin/perl -w

#****c* Cargo/Box
# FUNCTION
#   A box that can be packed into truck.
#   Several other classes are derived from Box.
#   Box
#   |
#   +---- SquareBox
#   |
#   +---- RectangularBox
#******

package Box;

sub new {
    my $class = shift;
    my $self = { };
    bless ($self, $class);
    return $self;
}

1;

