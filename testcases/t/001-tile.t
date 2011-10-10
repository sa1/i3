#!perl
# vim:ts=4:sw=4:expandtab

use i3test;
use X11::XCB qw(:all);

BEGIN {
    use_ok('X11::XCB::Connection');
    use_ok('X11::XCB::Connection::Rect');
}

my $x = X11::XCB::Connection->new;

my $original_rect = X11::XCB::Connection::Rect->new(x => 0, y => 0, width => 30, height => 30);

my $window = $x->root->create_child(
    class => WINDOW_CLASS_INPUT_OUTPUT,
    rect => $original_rect,
    background_color => '#C0C0C0',
);

isa_ok($window, 'X11::XCB::Connection::Window');

is_deeply($window->rect, $original_rect, "rect unmodified before mapping");

$window->map;

wait_for_map($x);

my $new_rect = $window->rect;
ok(!eq_deeply($new_rect, $original_rect), "Window got repositioned");

done_testing;
