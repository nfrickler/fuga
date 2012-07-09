#!/usr/bin/perl

# ####################################################################
# project:     FuGa v1.1
# copyright:   2012 Nicolas Frinker <authornicolas@tsunic.de>
# Description: This file offers the object FugaUser
# ####################################################################

use strict;
use warnings;

my $DEBUG = 1;

# ####################################################################
# FugaUser
# Handles a user
# ####################################################################

package FugaUser;
use DBI;
use Digest::SHA qw(sha1 sha1_hex);

sub new {
    my ($obj, $DB, $name, $logged) = @_;
    return undef unless $DB;

    # user exists?
    return undef if $name and !isUser(undef, $name, $DB);

    # create instance
    $DEBUG and print "FugaUser->new: Create new object\n";
    my $self = {
	DB => $DB,
	name => $name || '',
	logged => $logged || 0,
    };
    bless $self, 'FugaUser';
    return $self;
}

sub login {
    my ($self, $name, $password, $ip, $port) = @_;

    # name exists?
    if ($self->isUser($name)) {

	# is correct password?
	if ($self->isValidLogin($name, $password)) {
	    $self->{name} = $name;
	    $self->{logged} = 1;

	    # update database
	    my $sql = "
		UPDATE users
		SET dateOfLast = NOW(),
		    dateOfLogin = NOW(),
		    tcp_ip = '$ip',
		    tcp_port = '$port',
		    logged = 1
		WHERE name = '$name';";
	    $self->_db($sql, 1);

	    return 1;
	}

	return 0;
    }

    # valid name and password?
    return 0 unless $self->validName($name) and $self->validPassword($password);

    # create profile in database (locks name!)
    $password = sha1_hex($password);
    my $sql = "
	INSERT INTO users
	SET name = '$name',
	    password = '$password',
	    dateOfLast = NOW(),
	    dateOfRegistration = NOW(),
	    logged = 0
	ON DUPLICATE KEY UPDATE
	    password = '$password',
	    dateOfLast = NOW(),
	    dateOfRegistration = NOW(),
	    logged = 0;";
    $self->_db($sql, 1);

    return 2;
}

sub setPassword {
    my ($self, $password) = @_;
    return 0 unless $self->{logged};
    return 0 unless $self->validPassword($password);
    $password = sha1_hex($password);

    my $sql = "
	UPDATE users
	SET password = '$password'
	WHERE name = '".$self->{name}."'";
    return $self->_db($sql, 1);
}

sub logout {
    my $self = shift;
    return 0 unless $self->{logged};

    my $sql = "
	UPDATE users
	SET logged = 0
	WHERE name = '".$self->{name}."';";
    $self->_db($sql, 1);

    $self->{logged} = 0;
    $self->name(0);
    return 1;
}

sub delete {
    my $self = shift;
    return 0 unless $self->{logged};

    my $sql = "
	DELETE FROM users
	WHERE name = '".$self->{name}."';";
    my $result = $self->_db($sql, 1);
    return 0 unless $result;

    $self->{logged} = 0;
    $self->{name} = "";
    return 1;
}

sub name {
    my ($self) = @_;
    return $self->{name};
}

sub getInfo {
    my ($self, $name) = @_;
    $name = $self->{name} unless $name;

    # get data of this user
    my $sql = "
	SELECT name,
	    tcp_ip,
	    tcp_port,
	    dateOfRegistration,
	    dateOfLogin,
	    dateOfLast
	FROM users
	WHERE name = '".$name."';";
    return $self->_db($sql, 0);
}

sub getAll {
    my ($self) = @_;

    my $sql = "
	SELECT name,
	    tcp_ip,
	    tcp_port,
	    dateOfRegistration,
	    dateOfLogin,
	    dateOfLast
	FROM users
	ORDER BY name;";
    return $self->_db($sql, 0);
}

sub setTcp {
    my ($self, $ip, $port) = @_;
    return 0 unless $self->{logged};
    return 0 unless $ip and $port;

    # update database
    my $sql = "
	UPDATE users
	SET tcp_ip = '$ip',
	    tcp_port = '$port'
	WHERE name = '".$self->name()."';";
    $self->_db($sql, 1);

    return 1;
}

sub getTcp {
    my ($self) = @_;

    # request database
    my $sql = "
	SELECT tcp_ip, tcp_port
	FROM users
	WHERE name = '".$self->name()."';";
    my $result = $self->_db($sql, 0);
    return 0 unless $result and @$result;

    return ($result->[0]{tcp_ip}, $result->[0]{tcp_port});
}

# is this a valid user?
sub isValid {
    my $self = shift;
    return $self->{name} ? 1 : 0;
}

# ####################################################################
# Helper
# ####################################################################

sub isUser {
    my ($self, $name, $DB) = @_;
    $DB = $self->{DB} if $self and !$DB;
    my $request = "
	SELECT name
	FROM users
	WHERE name = '$name'
	    AND TIMESTAMPDIFF(MONTH, dateOfLogin, NOW()) < 6
	    OR (dateOfLogin = '0000-00-00 00:00:00'
		AND TIMESTAMPDIFF(MINUTE, dateOfLast, NOW()) < 2);";
    my $result = FugaUser->_db($request, 0, $DB);
    return (@$result) ? 1 : 0;
}

sub isValidLogin {
    my ($self, $name, $password) = @_;
    $name = $self->{name} unless $name;
    $password = sha1_hex($password);

    # check login data
    my $request = "
	SELECT name
	FROM users
	WHERE name = '$name'
	    AND password = '$password';";
    my $result = $self->_db($request, 0);
 
    return (@$result) ? 1 : 0;
}

# is valid name?
sub validateName {
    my ($self, $name) = @_;

    # is valid?
    return 1 if defined $name and $name =~ /^[a-zA-Z0-9_]{4,15}$/;
    return 0;
}

# validate password
sub validatePassword {
    my ($self, $passwd) = @_;

    return 1 if defined $passwd and $passwd =~ /^[a-zA-Z0-9_]+$/;
    return 0;
}

# ####################################################################
# Database handling
# ####################################################################

# request database
sub _db {
    my ($self, $request, $sendOnly, $DB) = @_;
    $DB = $self->{DB} if $self and !$DB;

    # send request
    $DEBUG and print "FugaUser->_db: sql: $request\n";
    my $sth = $DB->prepare($request) or die $DBI::errstr;
    $sth->execute() or die $DBI::errstr;

    # return?
    return 1 if $sendOnly;

    # handle return
    my @output;
    while (my $row = $sth->fetchrow_hashref()) {
	push @output, $row;
    }

    return @output ? \@output : [];
}

# is valid password?
sub validPassword {
    my ($self, $password) = @_;
    return ($password =~ /^.{4,}$/) ? 1 : 0;
}

# is valid name?
sub validName {
    my ($self, $name) = @_;
    return ($name =~ /^\w{4,}$/) ? 1 : 0;
}

1
