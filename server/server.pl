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
use FugaCrypto;
use Switch;
use DBI;
use Getopt::Long;

# get parameters
our %options;
GetOptions(
    "net=s"	=> \$options{net},
    "ip=s"	=> \$options{ip},
    "port|p=i"	=> \$options{port},
    "dbh=s"	=> \$options{db_host},
    "dbu=s"	=> \$options{db_user},
    "dbp=s"	=> \$options{db_password},
    "dbdb=s"	=> \$options{db_database},
    "help|h"	=> \$options{help},
);
$options{net}		||= "root";
$options{ip}		||= "127.0.0.1";
$options{port}		||= 7777;
$options{db_host}	||= "localhost";
$options{db_user}	||= "root";
$options{db_password}	||= "toortoor";
$options{db_database}	||= "fuga";

# print help?
if ($options{help}) {
    print <<HELP;
Usage: server.pl [options]

Options:
    --net    Network of this server ("root" starts root server)
    --ip     Ip address of this server
    --port   Port of this server
    --dbh    Host for database
    --dbu    User for database
    --dbp    Password for database
    --dbdb   Name of database
    --help   Show this help

HELP
    exit;
}

# ####################################################################
# Init
# ####################################################################

print "Server starting up...\n";
print "Start listening on ".$options{ip}.":".$options{port}."\n";

# get FugaCrypto
my $Crypto = FugaCrypto->new();
die "Could not start Crypto" unless $Crypto;

# get FugaServer
my $Server = FugaServer->new(
    $options{ip},
    $options{port},
    \&handleRequest,
    \&handleStdin,
);

# start server
$Server->start();

# connect to database
my $driver = "DBI:mysql:".$options{db_database}.":".$options{db_host};
my $DB = DBI->connect($driver, $options{db_user}, $options{db_password})
    or die("Could not connect to database");
print "Connected to database.\n";

# create table if not existing
open(my $dbfh, "<", "dbtable.sql") or die "Could not open database file!";
local $/ = undef;
my $request = <$dbfh>;
close($dbfh);
my $dbname = "fuga_".$options{net};
$request =~ s/#db#/$dbname/;
my $sth = $DB->prepare($request) or die $DBI::errstr;
$sth->execute() or die $DBI::errstr;

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
	    return "FAIL Invalid password!"
		unless $User->validPassword($password);
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
    $Conn->{User} = FugaUser->new($options{net},$DB) unless $Conn->{User};
    my $User = $Conn->{User};

    # handle request
    switch ($typeid) {
	case "r_sverify" {
	    return client_sverify($User,$Conn,$data);
	}
	case "r_login" {
	    return "a_login_failed-Invalid action" if $options{net} eq "root";
	    return client_login($User,$Conn,$data);
	}
	case "r_logout" {
	    return "a_login_failed-Invalid action" if $options{net} eq "root";
	    return client_logout($User);
	}
	case "r_name2tcp" {
	    return client_name2tcp($User,$data);
	}
	case "r_delete" {
	    return "a_login_failed-Invalid action" if $options{net} eq "root";
	    return client_deleteMe($User, $Conn, $data);
	}
    }

    return "0-Invalid";
}

# send verification message to client
sub client_sverify {
    my ($User, $Conn, $data) = @_;

    # message to sign
    my $msg2sign = $data."_".$options{net};

    # construct and send message
    my $msg = "a_sverify-" . $options{net} . "," . time() . "," .
	$Crypto->getPub() . "," .
	$Crypto->sign($msg2sign) . ";";
    chomp($msg);
    $Conn->write($msg);
}

# login
sub client_login {
    my ($User, $Conn, $data) = @_;
    my ($data2, $password, $ip, $port, $pubkey) = split ",", $data;
    my ($name, $network) = split "%", $data2;

    # are we the right network?
    return "a_login_failed-Wrong network" if $network ne $options{net};

    # try to login
    my $result = $User->login($name, $password, $ip, $port, $pubkey);
    return "a_login_failed" unless $result;
    return "a_login_confirm" if $result == 2;

    # success
    return "a_login_ok";
}

# get tcp data for name
sub client_name2tcp {
    my ($User, $data) = @_;
    my ($name, $network) = split "%", $data;
    $network ||= "root";

print "GOT client_name2tcp\n";
    # other network?
    if ($network ne $options{net}) {
	# TODO
	return "a_name2tcp_failed-Ask someone else!";
    }

print "GOT client_name2tcp 2\n";
    # get User
    my $Req = FugaUser->new($network, $DB, $name);
    return "a_name2tcp_failed-".$data.",No such user" unless $Req;
use Data::Dumper;
print Dumper($Req);
print "GOT client_name2tcp 3\n";
    # get data
    my ($ip, $port, $pubkey) = $Req->getTcp();
print "GOT client_name2tcp 4\n";
    return "a_name2tcp_failed-".$data.",No data"
	unless defined $ip and defined $port;
    return "a_name2tcp-$data,$ip,$port,$pubkey";
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
