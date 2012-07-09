#!/usr/bin/perl

# ####################################################################
# project:     FuGa v1.1
# copyright:   2012 Nicolas Frinker <authornicolas@tsunic.de>
# Description: Start this file to run the server.
# ####################################################################

use strict;
use warnings;

my $DEBUG = 1;

package FugaServer;
use FugaServer;
use FugaUser;
use Switch;
use DBI;

# get parameter
our $server_host = $ARGV[0] || "127.0.0.1";
our $server_port = $ARGV[1] || 7777;

# ####################################################################
# Init
# ####################################################################

print "Server starting up...\n";
print "Listening on $server_host port $server_port \n";

# get FugaServer
my $Server = FugaServer->new(
    $server_host,
    $server_port,
    \&handleRequest,
    \&handleStdin,
);

# start server
$Server->start();

# set parameters for database
my $user = 'root';
my $pass = "toortoor";
my $database = "fuga";
my $host = "localhost";
my $driver = "DBI:mysql:$database:$host";

# connect to database
my $DB = DBI->connect($driver, $user, $pass) or die("Could not connect to database");
print "Connected to database.\n";

# ####################################################################
# Run
# ####################################################################

print "Server ready.\n";
$Server->main();

# ####################################################################
# Handle inputs
# ####################################################################

# handle input from stdin
# @param command
sub handleStdin {
    my @input    = split /\s+/, shift;
    my $cmd      = $input[0] || "";
    my $name     = $input[1] || "";
    my $password = $input[2] || "";

    # get FugaUser
    my $User = FugaUser->new(
	$DB, (($name and $name ne "all") ? $name : undef), 1
    );
    return "FAIL Invalid user!\n" unless $User;

    # handle request
    switch ($cmd) {

	# delete 'name'
	case "delete" {
	    return "FAIL Got password unexpectedly!" if $password;
	    return "FAIL Invalid name!" unless $User->validName($name);
	    return $User->delete()
		? "OK User '$name' deleted" : "FAIL User could not be deleted";
	}

	# password 'name' 'newpassword'
	case "password" {
	    return "FAIL Invalid password!" unless $User->validPassword($password);
	    return "FAIL Invalid name!" unless $User->isValid();
	    return $User->setPassword($password)
		? "OK Password changed" : "FAIL Password could not be changed.";
	}

	# print 'name' | print all
	case "print" {
	    return admin_print_all() if $name eq "all";
	    return "FAIL Invalid name!" unless $User->validName($name);
	    return admin_print($name);
	}

	# exit
	case "exit" {
	    admin_close();
	}

	# help
	case "help" {

return <<MYHELP;

You can enter one of the following commands, to administer
the server and it's accounts.

[password 'name' 'password'] set new password for user
[delete 'name']	  delete user
[print all]	      print all registered users
[print 'name']	   print infos of user
[help]		   show this help
[exit]		   close server
MYHELP

	}
    }

    return "FAIL Invalid command!";
}

# print all users
sub admin_print_all {

    # get all Users in database
    my $User = FugaUser->new($DB);
    my $results = $User->getAll();
    return "No users found!" unless @$results;

    # print all users
    my $answer = "";
    for my $row (@$results) {
	$answer.= "\n";
	for (keys %$row) {
	    $answer.= " | ".$row->{$_};
	}
    }
    return $answer;
}

# print infos to user
sub admin_print {
    my $name = shift;

    # get info
    my $User = FugaUser->new($DB, $name);
    return "FAIL No such user!" unless $User;
    my $results = $User->getInfo();

    # print user
    my $answer = "";
    for my $row (@$results) {
	$answer.= "\n";
	for (keys %$row) {
	    $answer.= " | ".$row->{$_};
	}
    }
    return $answer;
}

# close server
sub admin_close {
    $DB->disconnect;
    $Server->closeServer();
    exit(0);
}

# ####################################################################
# Handle requests from clients
# ####################################################################

# handle request
# @param: Conn
# @param: TypeID
# @param: Request-Data
sub handleRequest {
    my ($Conn, $typeid, $data) = @_;
    $DEBUG and print "main->handleRequest: got '$typeid' and '$data'\n";

    # Connection to User
    $Conn->{User} = FugaUser->new($DB) unless $Conn->{User};
    my $User = $Conn->{User};

    # handle request
    switch ($typeid) {

	# r_login
	case "r_login" {
	    return client_login($User,$Conn,$data);
	}
	case "r_logout" {
	    return client_logout($User);
	}
	case "r_name2tcp" {
	    return client_name2tcp($User,$data);
	}
	case "r_delete" {
	    return client_deleteMe($User, $Conn, $data);
	}
    }

    return "0-Invalid";
}

# login
sub client_login {
    my ($User, $Conn, $data) = @_;
    my ($name, $password, $ip, $port) = split ",", $data;

    # try to login
    my $result = $User->login($name, $password, $ip, $port);
    return "a_login_failed" unless $result;
    return "a_login_confirm" if $result == 2;

    # success
    return "a_login_ok";
}

# get tcp data for name
sub client_name2tcp {
    my ($User, $data) = @_;

    # get User
    my $Req = FugaUser->new($DB, $data);
    return "a_name2tcp_failed-".$data.",No such user!" unless $Req;

    # get data
    my ($ip, $port) = $Req->getTcp();
    return "a_name2tcp_failed-".$data.",No data"
	unless defined $ip and defined $port;
    return "a_name2tcp-$data,$ip,$port";
}

# logout
sub client_logout {
    my ($User) = @_;
    $User->logout();
    return "a_logout_ok";
}

# delete user
sub client_deleteMe {
    my ($User, $Conn, $password) = @_;

    # is valid password?
    return "a_delete_fail-Wrong password!"
	unless $User->isValidLogin(undef, $password);

    # delete user
    my $result = $User->delete();
    return "a_delete_failed" unless $result;

    # remove object
    $Conn->{User} = undef;

    return "a_delete_ok";
}
