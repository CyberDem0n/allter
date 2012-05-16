package Common;
use strict;
use Exporter;
use vars qw/@ISA @EXPORT $VERSION/;
@ISA = ('Exporter');
$VERSION=1.0;

@EXPORT =qw/	&get_status
		&get_int_param
		&get_search_time
		&isEmpty
		&utime2str
		&printd
		&ip2nb_name
		&is_good_ip
		&IPAllowed
		&ADMIN_ADDRESS
		&WWWROOT/;

use Data::Dumper;

sub get_status {
	my $IP = shift;
	my %STATS = @_;
	return $STATS{$IP} if exists $STATS{$IP};
	return "unknown" unless($IP =~ /^(\d+\.\d+\.\d+)\.(\d+)$/);
	open(FILE,"/home/allter/pinger/testhost ".$IP."|");
	my $status = <FILE>;
	close FILE;
	$status = "unknown" if(length($status)!=1);
	if($status eq '0'){$STATS{$IP} = "na";}
	elsif($status eq '1'){$STATS{$IP} = "available";}
	else {$STATS{$IP} = "unknown";}
	return $STATS{$IP};
}

sub get_int_param {
	my $str = shift || 0;
	$str =~ s/^0+//g;
	return ($str =~ /^\d+$/)?$str:0;
}

sub isEmpty($) {
	my $str = shift || return 1;
	$str=~s/\s//g;
	return 1 unless length $str;
	return 0;
}

sub get_search_time($) {
	my $search_time = shift;
	$search_time =~ /^(\d+[\,|\.]\d{2})/;
	return $1;
}

sub utime2str($) {
	my $utime = shift;
	my ($sec,$min,$hour,$day,$mon,$year) = localtime $utime;
	$year %= 100; $mon++;
	return sprintf("%02d/%02d/%02d %02d:%02d:%02d", $day, $mon, $year, $hour, $min, $sec);
}

sub printd($) {
	my $num = shift;
	my $res = "";
	$res = " ".$1.$res while($num =~ s/(\d{3})$//g);
	$res = $num.$res;
	return $res;
}

sub ip2nb_name {
	my $IP = shift;
	open NL,"/usr/bin/nmblookup -A $IP|";
	my @lines = <NL>;
	close NL;
	my ($NAME,$GROUP) = ("","");
	foreach(@lines) {
		if($_ =~ /^\t(.+?)\s+<20>\s\-\s+/) {
			$NAME = $1 unless length $NAME;
		} elsif(/^\t(.+?)\s+<00> - <GROUP>/) {
			$GROUP = $1 unless length $GROUP;
		}
		last if(length $NAME && length $GROUP);
	}
	return ($NAME,$GROUP) if(length $NAME && length $GROUP);
	return ("","");
}

sub is_good_ip {
	my $IP = shift;
	my @bad_ips = ();

	foreach(@bad_ips) {
		return 0 if($_ eq $IP);
	}
	return 1;
}

sub IPAllowed {
        my $IP = shift;
        my @bad_ips = ();
	my @bad_nets = ();
	foreach(@bad_ips) {
		return 0 if($_ eq $IP);
	}
	foreach(@bad_nets) {
		return 0 if(substr($IP,0,length($_)) eq $_);
	}
	return 1;
}				

sub ADMIN_ADDRESS {
    return "127.0.0.1";
}

sub WWWROOT {
    return "/home/allter/www/";
}

1;
