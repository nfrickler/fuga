#!/usr/bin/perl

# ####################################################################
# project:     FuGa v1.1
# copyright:   2012 Nicolas Frinker <authornicolas@tsunic.de>
# Description: Offers the object FugaServer
#              The FugaServer object handles the server stuff,
#              administrates the connections and the server socket
# ####################################################################

use strict;
use warnings;

my $DEBUG = 1;

# ####################################################################
# FugaServer
# This object handles a server
# ####################################################################

package FugaServer;
use FugaConnection;
use IO::Socket;
use IO::Socket::SSL;
use IO::Select;

require Exporter;
our @ISA = qw(Exporter);
our @EXPORT_OK = qw(new startSocket wait printToAll printTo);

# create new FugaServer object
sub new {
    my $obj = shift;
    my $self = {
	host => shift,
	port => shift,
	connections => undef,
	socket => undef,
	to_read => undef,
	to_write => undef,
	handler => shift,
	handler_stdin => shift,
    };
    bless $self, 'FugaServer';
    return $self;
}

# ####################################################################
# Start server
# ####################################################################

sub start {
    my $self = shift;

    # start socket
    $self->{socket} = IO::Socket::SSL->new(
	LocalHost => $self->{host},
	LocalPort => $self->{port},
	Proto => 'tcp',
	Listen => 1,
	Reuse => 1,
	SSL_verify_mode => 0x00,
	SSL_passwd_cb => sub {return "bluebell"},
	Blocking => 0
    ) or die "unable to create socket: ", &IO::Socket::SSL::errstr, "\n";

    # start listening
    $self->{'to_read'} = new IO::Select();
    $self->{'to_read'}->add($self->{'socket'});
    $self->{'to_write'} = new IO::Select();
    $self->{'to_write'}->add($self->{'socket'});

    # add stdin/out as connection
    STDIN->blocking(0);
    $self->{connections}{\*STDIN} = FugaConnectionStdin->new(\*STDIN);
    $self->{'to_read'}->add(\*STDIN);

    return 1;
}

# ####################################################################
# Main loop
# ####################################################################

sub main {
    my ($self, $finish_only) = @_;
    $DEBUG and print "FugaServer->main: start\n";

    while (1) {

	# is sth to send?
	# if we have sth to send, set timeout
	my $timeout = undef;
	$timeout = (grep { $self->{connections}{$_}->buf_out() } keys $self->{'connections'})
	    ? 0.3 : undef;

	# finish only?
	return 1 if ($finish_only and !$timeout);

	# handle all requests
	for my $hndl ($self->{to_read}->can_read($timeout)) {

	    # new connection?
	    if ($hndl == $self->{socket}) {
	    $DEBUG and print "FugaServer->main: new connection\n";

		# add new connection
		my $new_hndl = $hndl->accept();
		eval{$new_hndl->blocking(0)};
		if ($@) {
		    print "FugaServer: Client disconnected immediately\n";
		    next;
		}
		$self->{'to_read'}->add($new_hndl);
		$self->{'to_write'}->add($new_hndl);

		# create new FugaConnection
		$self->{connections}{$new_hndl} =
		    FugaConnection->new($new_hndl);
		next;
	    }

	    # read all available data
	    $DEBUG and print "FugaServer->main: read from Connection\n";
	    unless ($self->{connections}{$hndl}->read()) {
		$DEBUG and print "FugaServer->main: Client disconnected.\n";
		$self->closeConnection($hndl);
		next;
	    }

	    # handle complete messages
	    while (my $msg = $self->{connections}{$hndl}->getMsg()) {
		my $return = $self->runLine($hndl, $msg);
		return 0 if ($return == 100);
		last unless $return;
	    }

	    # write
	    my @todo_write = $self->{'to_write'}->can_write(0.3);
	    for my $hndl (@todo_write) {
		next unless defined $self->{'connections'}{$hndl};

		# write output
		$self->{connections}{$hndl}->write();

		# close connection?
		$self->endClient($hndl, 1)
		    if ($self->{'clients'}{$hndl}{'wait_for_exit'});
	    }
	}
    }

    return 1;
}

# ####################################################################
# Receiving data
# ####################################################################

# handle input-line from client
sub runLine {
    my ($self, $hndl, $input) = @_;
    chomp($input);
    $input =~ s/\r//;
    $input =~ s/(^\s*)//;
    $input =~ s/(\s*$)//;
    return 1 unless ($input);

    # std?
    if ($hndl eq \*STDIN) {
	$DEBUG and print "FugaServer->runLine: Got input from STDIN\n";
	print $self->{handler_stdin}->($input), "\n";
	return 1;
    }

    # server-output
    $DEBUG and print "FugaServer->runLine: Got input from Connection: $input\n";

    # split input
    my ($typeid, $msgdata) = $input =~ /^(.*?)-(.*)$/ms;
    unless (defined $typeid and defined $msgdata) {
	print "# Received invalid request!\n";
	$self->printTo($hndl, "a_bad_request-;");
	return 1;
    }

    # run external...
    $self->printTo($hndl, $self->{handler}->(
	$self->{connections}{$hndl},
	$typeid,
	$msgdata
    ));

    return 1;
}

# ####################################################################
# Sending data
# ####################################################################

# print s.th. to all connections
sub printToAll {
    my ($self, $input) = @_;

    for my $curhndl (keys %{$self->{'clients'}}) {
	$self->printTo($curhndl, $input);
    }
}

# print to connection
sub printTo {
    my ($self, $hndl, $input) = @_;
    $self->{connections}{$hndl}->write($input);
}

# ####################################################################
# Close connection
# ####################################################################

# shut down server
sub closeServer {
    my $self = shift;

    # finish sending
    $self->main(1);

    # close all connections
    for my $hndl (keys %{$self->{'connections'}}) {
	$self->closeConnection($hndl);
    }

    # close server
    $self->{'socket'}->close();

    print "Server closed.\n";
    return 1;
}

# end connection to client
sub closeConnection {
    my ($self, $hndl) = @_;

    # close connection
    $self->{'to_read'}->remove($hndl);
    $self->{'to_write'}->remove($hndl);
    $self->{connections}{$hndl}->close();
}

1;
