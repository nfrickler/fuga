#!/usr/bin/perl

# ####################################################################
# project:     Fuga v1.1
# copyright:   2012 Nicolas Frinker <authornicolas@tsunic.de>
# Description: This file contains the signature and encryption foo
# ####################################################################

use strict;
use warnings;

package FugaCrypto;
use Crypt::OpenSSL::DSA;
use MIME::Base64  qw(encode_base64);
use Digest::SHA qw(sha1 sha1_hex);

# create new FugaCrypto object
sub new {

    # bless new object
    my $self = {};

    bless $self, "FugaCrypto";
    return $self;
}

# load key
sub loadKey {
    my ($self) = @_;
    my $file_priv = "certs/fugakey.priv";
    my $file_pub = "certs/fugakey.pub";

    # need new keys?
    unless (-f $file_priv and -f $file_pub) {
	print "FugaCrypto: Create new keys\n";
	my $dsa = Crypt::OpenSSL::DSA->generate_parameters( 1024 );
	$dsa->generate_key;
	$dsa->write_pub_key( $file_pub );
	$dsa->write_priv_key( $file_priv );
    }

    print "Onload keys...\n";
    $self->{privkey} = Crypt::OpenSSL::DSA->read_priv_key( $file_priv );
    $self->{pubkey} = Crypt::OpenSSL::DSA->read_pub_key( $file_pub );
    open(my $fh_pub, "<", $file_pub);
    local $/ = undef;
    $self->{pubkey_plain} = <$fh_pub>;
    close($fh_pub);
}

# returns own public key
sub getPub {
    my $self = shift;
    $self->loadKey() unless $self->{pubkey};
    return $self->{pubkey_plain};
    #return $self->{key}->get_public_key_string();
}

# sign message
sub sign {
    my ($self, $msg) = @_;
    $self->loadKey() unless $self->{pubkey};

    $msg= sha1($msg);
    my $sig = $self->{privkey}->sign($msg);
    my $ready = encode_base64($sig);

    return $ready;
}

# verify signature
sub verify {
    my ($self, $pubkey, $msg, $signature) = @_;
    return $pubkey->verify($msg, $signature);
}

# ####################### key handling ###########################

# save public key of person
sub saveKey {
    my ($self, $name, $key) = @_;



}

# get pubkey of person
sub getKeyOf {
    my ($self, $name) = @_;

    return "";
}


1;
