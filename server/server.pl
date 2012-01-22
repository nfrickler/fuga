#!/usr/bin/perl
use strict;
use warnings;

package myserver;
use Digest::SHA qw(sha1 sha1_hex);
use ServerObject;
use Switch;
use DBI;

our $server_host = $ARGV[0] || "127.0.0.1";
our $server_port = $ARGV[1] || "7777";


# ###################### init ###################### #

print "Server started...\n";
print "Listening on $server_host port $server_port \n";

# start server-object
my $Server = ServerObject->new();

# start server
$Server->startSocket();

my $user = 'root';
my $pass = "toortoor";
my $database = "myserver";
my $host = "localhost";
my $driver = "DBI:mysql:$database:$host";

#connect to database
my $db = DBI->connect($driver, $user, $pass) or die("Could not connect to database");
print "Connected to database.\n";


# ###################### run ####################### #

print "Server entered running mode...\n";
$Server->wait();

# handle input from stdin
# @param command
sub handleStdin {
	my @input 	= 	split /\s+/, shift;
	my $cmd 	=	$input[0] || "";
	my $name 	=	$input[1] || "";
	my $passwd 	=	$input[2] || "";

	# handle request
	switch ($cmd) {

		# delete 'name'
		case "delete" {
			return "Got password unexpectedly!" if validatePassword($passwd);
			return "Invalid name!" unless validateName($name);
			return delete_usr($name);
		}

		# add 'name' 'passwd'
		case "add" {
			return "Invalid password!" unless validatePassword($passwd);
			return "Invalid name!" unless validateName($name);
			return add_usr($name, $passwd);
		}

		# passwd 'name' 'newpasswd'
		case "passwd" {
			return "Invalid password!" unless validatePassword($passwd);
			return "Invalid name!" unless validateName($name);
			return passwd($name, $passwd);
		}

		# print 'name' | print all
		case "print" {
			return print_all() if $name eq "all";
			return "Invalid name!" unless validateName($name);
			return print_usr($name);
		}

		# exit
		case "exit" {
			closing();
		}

		# help
		case "help" {

return <<MYHELP;

You can enter one of the following commands, to administer
the server and it's accounts.

[add 'name' 'passwd']    add new user
[passwd 'name' 'passwd'] set new password for user
[delete 'name']          delete user
[print all]              print all registered users
[print 'name']           print infos of user
[help]                   show this help
[exit]                   close server
MYHELP

		}
	}

	return "Invalid command!";
}

# handle request
# @param: name
# @param: TypeID
# @param: Request-Data
sub handleRequests {
	my ($name, $typeid, $data) = @_;

	# handle request
	switch ($typeid) {

		# r_login
		case "r_login" { 
			return login($name,$data);
		}
		case "r_logout" { 
			return logout($name);
		}
		case "a_mytcp" {
			return mytcp($name, $data);
		}
		case "r_name2tcp" {
			return name2tcp($name,$data);	
		}
		case "r_add_usr" {
			return add_usr_reg($data);
		}
	}

	return "0-Invalid";
}


# #################### handle admin commands ################# #

# add user
sub add_usr {
	my ($name, $passwd) = @_;
	$passwd = sha1_hex($passwd);

	# name already in use?
	return "Name already in use!" if isUser($name);

	# add user in database
	my $request_add = "INSERT INTO users
						SET name = '$name',
							pwd = '$passwd';";
	db_request($request_add, 1);

	return "User '$name' added.";
}

# change password
sub passwd {
	my ($name, $passwd) = @_;
	$passwd = sha1_hex($passwd);

	# name already in use?
	return "User doesn't exist!" unless isUser($name);

	# change password in database
	my $request = "UPDATE users
					SET pwd = '$passwd'
					WHERE name = '$name';";
	db_request($request, 1);

	return "Password for user '$name' changed.";
}

# print all users
sub print_all {

	# get all names from database
	my $request= "SELECT name, lastOn FROM users ORDER BY name";
	my $results = db_request($request, 0);
	return "No users found!" unless @$results;

	# print all users
	my $answer = "";
	$answer.= " $_->[1] : $_->[0]\n" for (@$results);
	return $answer;
}

# print infos to user
sub print_usr {
	my $name = shift;

	# get all names from database
	my $request= "SELECT name, lastOn FROM users WHERE name='$name';";
	my $results = db_request($request, 0);
	return "User not found!" unless @$results;

	# print user 
	my $answer = "";
	$answer.= " $_->[1] : $_->[0]\n" for (@$results);
	return $answer;
}

# delete user
sub delete_usr {
	my $name = shift;

	# delete user
	my $request= "DELETE FROM users
					WHERE name = '$name'";
	return "Database error!" unless db_request($request, 1);
	
	return "User '$name' deleted";
}

# close server
sub closing {
	$db->disconnect;
	$Server->closeServer();
	exit(0); 
}


# #################### handle client requests ################ #

# user registration
sub add_usr_reg {
	my ($name, $passwd) = split /,/, shift || "";

	# validate password
	return "a_add_failed" unless validatePassword($passwd);
	$passwd = sha1_hex($passwd);

	# name already in use?
	return "a_add_failed" if isUser($name);
	
	# add user in database
	my $request= "INSERT INTO users
					SET name = '$name',
						pwd = '$passwd';";
	db_request($request, 1);

	return "a_add_ok";
}

# login
sub login {
	my $oldname = shift;
	my ($login_usr, $login_passwd) = split /,/, shift || "";
	$login_passwd = sha1_hex($login_passwd);

	# is valid login_name?
	return "a_login_failed-Invalid name" unless validateName($login_usr);

	# request database
	my $request = "SELECT lastOn
					FROM users
					WHERE name = '$login_usr'
						AND pwd = '$login_passwd';";
	my $result = db_request($request, 0);
	return "a_login_failed" unless @$result; 

	# update database
	my $request_update = "UPDATE users SET lastOn = NOW() WHERE name = '$login_usr'";
	return "a_login_failed" unless db_request($request_update, 1);

	# set name
	$Server->setName($oldname, $login_usr);
	return "a_login_ok";
}

# update tcp-data
sub mytcp {
	my $name = shift;
	my ($tcpip, $tcpport) = split /,/, shift || "";

	# update database
	my $request = "UPDATE users
					SET tcp_ip = '$tcpip',
						tcp_port = '$tcpport'
					WHERE name = '$name'";
	db_request($request, 1);
	return "";
}

# get tcpdata for name
sub name2tcp {
	my ($name, $data) = @_;

	# request database
	my $request= "SELECT tcp_ip, tcp_port 
					FROM users
					WHERE name = '$data'";
	my $results = db_request($request, 0);
	return "a_name2tcp-failed" unless @$results;

	return "a_name2tcp-$data,@$results[0]->[0],@$results[0]->[1]";
}

# logout
# XXX Currently not in use!
sub logout {
	my $name = shift;

	# update database
	my $request = "UPDATE users
					SET lastOn = 0
					WHERE name = '$name'";
	return "a_login_failed" unless db_request($request, 1); 

	return "a_logout_ok";
}


# #################### helpers ############################### #

# is user?
sub isUser {
	my $name = shift;

	my $request = "SELECT ID FROM users WHERE name = '$name';";
	my $result = db_request($request, 0);
	return 1 if @$result;

	return 0;
}

# is valid name? 
sub validateName {
	my $name = shift;

	# is valid?
	return 1 if (defined $name
					and $name =~ /^[a-zA-Z0-9_]{4,15}$/);
	return 0;
}

# validate password
sub validatePassword {
	my $passwd = shift;

	return 1 if (defined $passwd
					and $passwd =~ /^[a-zA-Z0-9_]+$/);
	return 0;
}

# request database
sub db_request {
	my ($request, $sendOnly) = @_;

	# send request
	my $sth = $db->prepare($request) or die $DBI::errstr;
	$sth->execute() or die $DBI::errstr;

	# return?
	return 1 if $sendOnly;

	# handle return
	my @output;
	while (my @row = $sth->fetchrow_array()) {
		push(@output, \@row);
	}

	return \@output;
}

