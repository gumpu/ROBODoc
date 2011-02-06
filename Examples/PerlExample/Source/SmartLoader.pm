#!/usr/bin/perl -w

#****c* Loader/SmartLoader
# FUNCTION
#   This class implements an O(1) packing
#   algorithm.
# ATTRIBUTES
#   CARGO -- the cargo to be packed.
#******

Package SmartLoader;

sub new {
    my $class = shift;
    my $self = { };
    bless ($self, $class);
    my $self->{CARGO} = ();
    return $self;
}

#****m* Loader/SmartLoader::pack
# FUNCTION
#   A O(1) packing algorithm.
# SYNOPSIS
#   my $sequence = $packref->pack();
# RETURN VALUE
#   A squence that specifies how to pack the truck.
#******

sub pack {
    return 0;
}

1;
