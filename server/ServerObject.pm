#!/usr/bin/perl
use strict;
use warnings;

package ServerObject;
use IO::Socket;
use IO::Select;
require "server.pl";

require Exporter;
our @ISA = qw(Exporter);
our @EXPORT_OK = qw(new startSocket wait printToAll printTo);

# new
# create new ServerObject
sub new {
	my $obj = {	'clients' => undef,
				'socket' => undef,
				'to_read' => undef,
				'to_write' => undef,
				'accept' => 1
	};
	bless $obj, 'ServerObject';
	return $obj;
}

# startSocket
# start new server
sub startSocket {
	my $self = shift;

	# start socket
	$self->{'socket'} = new IO::Socket::INET(LocalHost => $myserver::server_host,
									LocalPort => $myserver::server_port,
									Proto => 'tcp',
									Listen => 1,
									Reuse => 1,
									Blocking => 0);
	die "Could not open socket ($!)!\n" unless ($self->{'socket'});

	# start listening
	$self->{'to_read'} = new IO::Select();
	$self->{'to_write'} = new IO::Select();
	$self->{'to_read'}->add($self->{'socket'});
	$self->{'to_write'}->add($self->{'socket'});

	# add stdin/out to clients
	STDIN->blocking(0);
	$self->{'clients'}{\*STDIN} = { name => "std",
						buffer => '',
						buffer_out => '',
						wait_for_exit => 0,
						handle => \*STDIN};
	$self->{'to_read'}->add(\*STDIN);

	return 1;
}

# wait
# wait for new input
sub wait {
	my ($self, $finish_only) = @_;

	while (1) {

		# is sth to send?
		my $timeout = undef;
		for my $hndl (keys %{$self->{'clients'}}) {
			if ($self->{'clients'}{$hndl}{'buffer_out'}) {
				$timeout = 0.3;
				last;
			}
		}

		# finish only?
		return 1 if ($finish_only and !$timeout);

		# select all current requests
		my @todo_read = $self->{'to_read'}->can_read($timeout);

		# handle all requests
		for my $hndl (@todo_read) {

			# new client?
			if ($hndl == $self->{'socket'}) {

				# refuse connection
				next unless $self->{'accept'};

				# add client
				my $new_hndl = $hndl->accept();
				$new_hndl->blocking(0);
				$self->{'to_read'}->add($new_hndl);
				$self->{'to_write'}->add($new_hndl);
	
				# add client to clients
				$self->startClient($new_hndl);

				next;
			}

			# read all available data from client
			my $chars = '';
			my $buffer = '';
			while (sysread($hndl, $chars, 1)) {
				$buffer.= $chars;
			}

			# close connection, if empty buffer
			$self->endClient($hndl, 1) unless $buffer;

			# get complete buffer
			$self->{'clients'}{$hndl}{'buffer'}.= $buffer;

			# handle client-request, if complete message
			if ($self->{'clients'}{$hndl}{'buffer'} =~ /;/
				or (defined $self->{'clients'}{$hndl}{'name'} 
			        and $self->{'clients'}{$hndl}{'name'} eq "std"
					and $self->{'clients'}{$hndl}{'buffer'} =~ /^/)) {

				# split input into commands 
				my @lines;
				if (defined $self->{'clients'}{$hndl}{'name'}
						and $self->{'clients'}{$hndl}{'name'} eq "std") {
					@lines = split(/^/, $self->{'clients'}{$hndl}{'buffer'});
					$self->{'clients'}{$hndl}{'buffer'} = "";
				} else {
					$self->{'clients'}{$hndl}{'buffer'}.= " " if $self->{'clients'}{$hndl}{'buffer'} =~ /;$/;
					@lines = split(/;/, $self->{'clients'}{$hndl}{'buffer'});
					$self->{'clients'}{$hndl}{'buffer'} = pop(@lines);
				}

				# process each line
				my $return;
				for (@lines) {
					$return = $self->runLine($hndl, $_);
					return 0 if ($return == 100);
					last unless $return;
				}
			}
		}

		# write to clients
		my @todo_write = $self->{'to_write'}->can_write(0.3);
		for my $hndl (@todo_write) {
			next unless defined $self->{'clients'}{$hndl};

			# is output for client?
			if ($self->{'clients'}{$hndl}{'buffer_out'}) {
				$self->{'clients'}{$hndl}{'buffer_out'} =~ s/[\n\s\r]*$//;
				print "# Send to '", $self->{'clients'}{$hndl}{'name'}, "': '",$self->{'clients'}{$hndl}{'buffer_out'},"'\n";

				# print
				print $hndl $self->{'clients'}{$hndl}{'buffer_out'};
				$self->{'clients'}{$hndl}{'buffer_out'} = '';
			}

			# close connection?
			$self->endClient($hndl, 1) if ($self->{'clients'}{$hndl}{'wait_for_exit'});
		}
	}

	return 1;
}

# startClient
# start connection to client
sub startClient {
	my ($self, $hndl) = @_;

	# add client to clients
	$self->{'clients'}{$hndl} = { name => undef,
						buffer => '',
						buffer_out => '',
						wait_for_exit => 0,
						handle => $hndl};

	return 1;
}

# runLine
# handle input-line from client
sub runLine {
	my ($self, $hndl, $input) = @_;
	chomp($input);
	$input =~ s/\r//;
	$input =~ s/(^\s*)//;
	$input =~ s/(\s*$)//;
	return 1 unless ($input);

	# is name?
	unless (defined $self->{'clients'}{$hndl}{'name'}) {

		my $newname;
		my $counter = 0;
		while ($self->isName($newname)) {
			$newname = "unknown_$counter";
			$counter++;
		}

		# set temporary name
		$self->{'clients'}{$hndl}{'name'} = $newname;
	}

	# std?
	if ($self->{'clients'}{$hndl}{'name'} eq "std") {
		print myserver::handleStdin($input), "\n";
		return 1;
	}

	# server-output
	print "# Received from '", $self->{'clients'}{$hndl}{'name'}, "': '$input'\n";

	# split input
	$_ = $input;
	my ($typeid, $msgdata) = /^([^-;]*)-(.*)$/;
	unless (defined $typeid and defined $msgdata) {
		print "# Received invalid request!\n";
		$self->addToBuffer($hndl, "a_bad_request-;");
		return 1;
	}

	# run external...
	$self->addToBuffer($hndl, myserver::handleRequests(	$self->{'clients'}{$hndl}{'name'},
														$typeid,
														$msgdata)
	);

	return 1;
}

# set name
sub setName {
	my ($self, $oldname, $newname) = @_;

	# is valid name
	my $hndl = $self->name2hndl($oldname);
	if ($self->isName($newname) or !$hndl) {
		return 0;
	}

	# save name
	return 0 if ($self->isName($newname));

	$self->{'clients'}{$hndl}{'name'} = $newname;

	return 1;
}

# isName
# does name exist?
sub isName {
	my ($self, $name) = @_;

	# is invalid name?
	unless (defined $name
			and $name
			and $name !~ /^\s*$/
	) {
		return 2;
	}

	my $return = 0;
	for my $curhndl (keys %{$self->{'clients'}}) {
		$return = 1 if (defined $self->{'clients'}{$curhndl}{'name'}
						and $self->{'clients'}{$curhndl}{'name'} eq $name);
	}

	return $return;
}

# name2hndl
# get hndl for name
sub name2hndl {
	my ($self, $name) = @_;
	return 0 unless $name;

	for my $curhndl (keys %{$self->{'clients'}}) {
		next unless defined $self->{'clients'}{$curhndl};
		next unless defined $self->{'clients'}{$curhndl}{'name'};
		return $curhndl if ($self->{'clients'}{$curhndl}{'name'} eq $name);
	}

	return 0;
}

# printToAll
# print s.th. to all clients
sub printToAll {
	my ($self, $input) = @_;

	for my $curhndl (keys %{$self->{'clients'}}) {
		next if $self->{'clients'}{$curhndl}{'wait_for_exit'};
		$self->addToBuffer($curhndl, $input);
	}

	return 1;
}

# printTo
# print to person
sub printTo {
	my ($self, $name, $input) = @_;
	my $hndl = $self->name2hndl($name);

	# add to buffer
	$self->addToBuffer($hndl, $input);

	return 1;
}

# getAllNames
# get all names of clients
sub getAllNames {
	my $self = shift;

	my @names;
	for my $curhndl (keys %{$self->{'clients'}}) {
		next unless ($self->{'clients'}{$curhndl}{'name'});
		push @names, $self->{'clients'}{$curhndl}{'name'};
	}

	return @names;
}

# refuse
# set, if new connections shall 
sub refuse {
	my ($self, $refuse) = @_;

	# set
	$self->{'accept'} = ($refuse) ? 0 : 1;

	return 1;
}

# closeServer 
# shut down server
sub closeServer {
	my $self = shift;

	# finish sending
	$self->wait(1);

	# close all connections
	for my $hndl (keys %{$self->{'clients'}}) {
		next unless defined $self->{'clients'}{$hndl};
		$self->endClient($hndl,1);
	}

	# close server
	$self->{'socket'}->close();

	print "Server closed.\n";
	return 1;
}

# endClient
# end connection to client
sub endClient {
	my ($self, $hndl, $force) = @_;

	if ($force) {
		# inform others, that this person has quit
		#$self->runLine($hndl, "r_logout-;");
		$hndl = $self->{'clients'}{$hndl}{'handle'};
		return 3 unless defined $hndl;

		# delete from clients
		$self->{'to_read'}->remove($hndl);
		$self->{'to_write'}->remove($hndl);
		$hndl->close();
		$self->{'clients'}{$hndl} = undef;
		delete $self->{'clients'}{$hndl};

		print "Client is exiting...\n";
		return 2;
	}

	# softly close connection
	$self->{'clients'}{$hndl}{'wait_for_exit'} = 1;

	# print status
	return 1;
}

# addToBuffer
# add sth to output-buffer
sub addToBuffer {
	my ($self, $hndl, $output) = @_;

	# correct message if neccessary
	$output.= "- " if ($output !~ /-/); 
	$output.= ";" if ($output !~ /;$/); 

	# add to buffer
	$self->{'clients'}{$hndl}{'buffer_out'}.= $output."\n";
	return 1;
}

1;
