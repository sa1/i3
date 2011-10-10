#!perl
# vim:ts=4:sw=4:expandtab
# Regression: Check if the focus stays the same when switching the layout
# bug introduced by 77d0d42ed2d7ac8cafe267c92b35a81c1b9491eb
use i3test;
use X11::XCB qw(:all);

BEGIN {
    use_ok('X11::XCB::Connection');
}

my $i3 = i3(get_socket_path());
my $x = X11::XCB::Connection->new;

sub check_order {
    my ($msg) = @_;

    my @ws = @{$i3->get_workspaces->recv};
    my @nums = map { $_->{num} } grep { defined($_->{num}) } @ws;
    my @sorted = sort @nums;

    cmp_deeply(\@nums, \@sorted, $msg);
}

my $tmp = fresh_workspace;

my $left = open_window($x);
my $mid = open_window($x);
my $right = open_window($x);

sync_with_i3($x);

diag("left = " . $left->id . ", mid = " . $mid->id . ", right = " . $right->id);

is($x->input_focus, $right->id, 'Right window focused');

cmd 'focus left';

is($x->input_focus, $mid->id, 'Mid window focused');

cmd 'layout stacked';

is($x->input_focus, $mid->id, 'Mid window focused');

done_testing;
