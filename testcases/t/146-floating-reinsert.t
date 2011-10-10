#!perl
# vim:ts=4:sw=4:expandtab
#
use X11::XCB qw(:all);
use i3test;

BEGIN {
    use_ok('X11::XCB::Connection');
}

my $x = X11::XCB::Connection->new;

my $tmp = fresh_workspace;

my $left = open_window($x);
my $mid = open_window($x);

cmd 'split v';
my $bottom = open_window($x);

my ($nodes, $focus) = get_ws_content($tmp);

#############################################################################
# 1: open a floating window, get it mapped
#############################################################################

# Create a floating window
my $window = open_floating_window($x);
ok($window->mapped, 'Window is mapped');

($nodes, $focus) = get_ws_content($tmp);
is(@{$nodes->[1]->{nodes}}, 2, 'two windows in split con');

#############################################################################
# 2: make it tiling, see where it ends up
#############################################################################

cmd 'floating toggle';

my ($nodes, $focus) = get_ws_content($tmp);

is(@{$nodes->[1]->{nodes}}, 3, 'three windows in split con after floating toggle');

done_testing;
