#!/usr/bin/perl

# ####################################################################
# project:     FuGa v1.1
# copyright:   2012 Nicolas Frinker <authornicolas@tsunic.de>
# Description: Objects FugaConnection and FugaConnectionStdin
#              Both objects handle connections
# ####################################################################

use strict;
use warnings;

my $DEBUG = 1;

# ####################################################################
# FugaConnection
# This object handles a single connection to a client
# ####################################################################

package FugaConnection;

require Exporter;
our @ISA = qw(Exporter);

# create new Connection object
sub new {
    my $obj = shift;
    my $self = {
	socket => shift,
	buf_in => '',
	buf_out => '',
	wait4exit => 0,
    };
    bless $self, 'FugaConnection';
    return $self;
}

# handle buf_out
sub buf_out {
    my ($self, $input) = @_;
    $self->{buf_out} = $input if $input;
    return $self->{buf_out};
}

# handle buf_in
sub buf_in {
    my ($self, $input) = @_;
    $self->{buf_in} = $input if $input;
    return $self->{buf_in};
}

# handle socket
sub socket {
    my ($self, $input) = @_;
    $self->{socket} = $input if $input;
    return $self->{socket};
}

# handle read from socket
sub read {
    my $self = shift;

    # read all from socket
    my $chars = '';
    my $buffer = '';
    while (sysread($self->{socket}, $chars, 1)) {
	$buffer.= $chars;
    }

    # close connection if nothing to read
    unless ($buffer) {
	return 0;
    }

    # join buffers
    $self->{buf_in}.= $buffer;

    return 1;
}

# read next message (if available)
sub getMsg {
    my $self = shift;
    $self->{buf_in} =~ s/^[\n\r\s]*(.*?);[\n\r\s]*//ms;
    return $1 ? $1 : '';
}

# handle write
sub write {
    my ($self, $input) = @_;

    # add input to buffer
    if ($input) {
	$input.= '- ' unless $input =~ /-/;
	$input.= ';' unless $input =~ /;$/;
	$self->{buf_out}.= $input;
    }

    # write to socket
    my $socket = $self->{socket};
    $self->{buf_out} =~ s/[\n\s\r]*$//;
    print $socket $self->{buf_out};
    $DEBUG and print "FugaConnection: Sending: ".$self->{buf_out};

    # clear buffer
    $self->{buf_out} = '';
}

# close connection
sub close {
    my ($self,$nowrite) = @_;

    # write out_buf
    $self->write() unless $nowrite;

    $self->{socket}->close();
    $self = undef;
}

# ####################################################################
# FugaConnectionStdin
# This object handles a connection to STDIN
# ####################################################################

package FugaConnectionStdin;
use base 'FugaConnection';

sub new {
    my $obj = shift;
    my $self = {
	socket => shift,
	buf_in => undef,
	buf_out => undef,
	wait4exit => 0,
    };
    bless $self, 'FugaConnectionStdin';
    return $self;
}

sub write {
    # do nothing
}

sub close {
    # do nothing
}

# read next message (if available)
sub getMsg {
    my $self = shift;
    $self->{buf_in} =~ s/^(.*?)[\n\r]+//;
    return $1 ? $1 : '';
}

1;
