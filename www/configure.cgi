#!/usr/bin/perl
use strict;
use locale;
use POSIX;
use CGI;
use HTML::Template;
use Data::Dumper;

use vars qw/$default_update_interval/;

BEGIN {
	push @INC,'/home/allter/modules';
	setlocale(LC_ALL,"ru_RU.KOI8-R");
}
use Common;
use SQL;

&main;

exit;

sub main {
	my $SQL = new SQL;
	my $Q = new CGI;
	if (&IPAllowed($ENV{REMOTE_ADDR})) {
  		my $Template = HTML::Template->new(filename => 'tmpls/configure.html');
		$Template->param(REMOTE_ADDR=>($ENV{REMOTE_ADDR}=$Q->param('REMOTE_ADDR'))) if($ENV{REMOTE_ADDR} eq &ADMIN_ADDRESS && $Q->param('REMOTE_ADDR'));
		print "Content-type: text/html;charset=koi8-r\n\n";
		my ($db_cuid,$db_cname,$db_callow_scan) = &get_comp_info($SQL);
		my $comp_name = $Q->param('comp_name');
		$comp_name =~ s/[<>\\\/&;"]//g;

		$default_update_interval = 21600;
	
		if($Q->param('action')) {
			if($Q->param('action') eq 'update') {
				($db_cuid,$db_cname,$db_callow_scan) = &update_comp_info($db_cuid,$comp_name,$Q,$SQL,$Template);
				&update_shares_info($Q,$SQL);
			}
	  	} 

		$Template->param(comp_ip=>$ENV{REMOTE_ADDR});
		$Template->param(comp_name=>$db_cname);
		$Template->param(allow_scan=>$db_callow_scan);

		if($db_cuid) {
			$Template->param(shares_loop=>&show_shares($db_cuid,$db_cname,$SQL));
		}
		print $Template->output;
	}
	else {
		my $Template = HTML::Template->new(filename => 'tmpls/confdenied.html');
        	print "Content-type: text/html;charset=koi8-r\n\n";
		print $Template->output;
	}
}

sub str2seconds {
	my $str = shift;
	my ($t1,$t2);
	if($str =~ /^(\d+)(\w)$/) {
		$t1 = $1; $t2 = $2;
		return $t1*3600 if($t2 eq 'h' && ($t1==6 || $t1==12));
		return $t1*3600*24 if($t2 eq 'd' && $t1>=1 && $t1<=3);
		return $t1*3600*24*7 if($t2 eq 'w' && $t1>=1 && $t1<=2);
		return $t1*3600*24*30 if($t2 eq 'm' && $t1==1);
	}
	return $default_update_interval;
}

sub update_shares_info {
	my ($Query,$SQL) = @_;
	my ($cuid,$suid);
	my ($sth,$ref);
	foreach($Query->param) {
		if(/^share_int(\d+)$/) {
			$suid = $1;
			$ref = $SQL->sOneBase(fields=>['cip'],table=>'hosts,shares',where=>"shares.uid=$suid AND host=hosts.uid");
			if($#{$ref}!=-1) {
				next if($ref->[0][0] ne $ENV{REMOTE_ADDR});
				$SQL->UPDATE("UPDATE shares set update_interval=".&str2seconds($Query->param($_)).", allow_scan=".($Query->param('share_ch'.$suid)?1:0)." WHERE uid=$suid");
			}
		}
	}
}

sub get_update_interval {
	my $tm = shift;
	my $temp = int($tm/3600);
	return $temp.'h' if($temp==6 || $temp==12);
	$temp = int($tm/(3600*24));
	return $temp.'d' if($temp>=1 && $temp<=3);
	$temp = int($tm/(3600*24*7));
	return $temp.'w' if($temp==1 || $temp==2);
	$temp = int($tm/(3600*24*30));
	return $temp.'m' if($temp==1);
	return '6h';
}

sub show_shares {
	my ($db_cuid,$comp_name,$SQL) = @_;
	my $db_shares;
	my @real_shares;
	my @shares_loop;
	@real_shares = &get_shares(name=>$comp_name,ip=>$ENV{REMOTE_ADDR});
	$db_shares = $SQL->sOneBase(fields=>['uid,name','allow_scan','update_interval'],table=>'shares',where=>"host=$db_cuid");
	my $i;
	foreach my $db_share(@{$db_shares}) {
		push @shares_loop,{share_name=>$db_share->[1],
		uid=>$db_share->[0],allow_scan=>$db_share->[2],
		&get_update_interval($db_share->[3])=>1};
	}
	foreach my $real_share(@real_shares) {
		my $found = 0;
		foreach my $db_share(@{$db_shares}) {
			if(uc($db_share->[1]) eq uc($real_share)) {
				$found = 1;
				last;
			}
		}
		unless($found) {
			push @shares_loop,{share_name=>$real_share,
			uid=>$SQL->insert_share(name=>$real_share,parent=>$db_cuid,update_interval=>$default_update_interval),
			allow_scan=>1,&get_update_interval($default_update_interval)=>1};
		}
	}
	return \@shares_loop;
}


sub get_comp_info {
	my $SQL = shift;
	my $temp = $SQL->sOneBase(fields=>['uid','cname','allow_scan'],table=>'hosts',where=>"cip='".$ENV{REMOTE_ADDR}."'");
	my ($db_cuid,$db_cname,$db_callow_scan,$db_workgroup) = (0,'',0,'');
	if($#{$temp}!=-1) {($db_cuid,$db_cname,$db_callow_scan) = @{$temp->[0]};}
	else {
		($db_cname,$db_workgroup) = &ip2nb_name($ENV{REMOTE_ADDR});
		if(length $db_cname) {
			$db_cuid = $SQL->insert_comp(cname=>$db_cname,cip=>$ENV{REMOTE_ADDR},workgroup=>$db_workgroup,allow_scan=>1);
			$db_callow_scan = 1;
		}
	}
	return ($db_cuid,$db_cname,$db_callow_scan);
}


sub get_shares {
	my %host = @_;
	my $host = \%host;
	my($i,$parse,$temp) = (0,0,'');
	my @out;
	open SM,"/usr/bin/smbclient -U guest -N -L \"$host->{name}\" ".($host->{ip}?"-I $host->{ip}":"")."|";
	my @lines = <SM>;
	close SM;
	for($i=0;$i<@lines-1;$i++) {
		if($lines[$i] =~ /Sharename\s+Type\s+Comment/i
		&& $lines[$i+1] =~ /\-{9}\s+\-{4}\s+\-{7}/) {$i+=2;$parse=1;}
			last if($lines[$i] =~ /^[\t\s\n]+$/ && $parse);
			if($parse) {
	    		$lines[$i] =~ s/^(.{26}).+/$1/g;
				if($lines[$i] =~ /^[\t\s]+(.+?)\s+Disk\s+$/) {
				push @out,$1;
			}
		}
	}
	return @out;
}

sub ip2nb_name {
	my $IP = shift;
	open NL,"/usr/bin/nmblookup -A $IP|";
	my @lines = <NL>;
	close NL;
	foreach(@lines) {
		return $1 if($_ =~ /^\t(.+?)\s+<20>\s\-.+/);
	}
	return '';
}

sub update_comp_info {
	my ($db_cuid,$comp_name,$Query,$SQL,$Template) = @_;
	my ($db_cname,$db_callow_scan) = ('',0);
	if(!$db_cuid && $comp_name) {
		unless($comp_name =~ /;/) {
			open SM,"/usr/bin/smbclient -U guest -N -L \"".$comp_name."\" -I $ENV{REMOTE_ADDR}|";
			my @lines = <SM>;
			close SM;
			foreach(@lines) {
				if(/Sharename/) {
					$db_cuid = $SQL->insert_comp(cname=>$comp_name,cip=>$ENV{REMOTE_ADDR},allow_scan=>$Query->param('allow_scan')?1:0);
					$db_cname = $comp_name;
					$db_callow_scan = $Query->param('allow_scan')?1:0;
			    }
			}
			$Template->param(error=>'Ваш компьютер не был занесен в базу по причине его недоступности. Возможно вы неверно ввели его имя.') unless($db_cuid);
	    }
	}
	elsif($comp_name) {
		unless($comp_name =~ /;/) {
			$db_cname = $comp_name;
			$db_callow_scan = $Query->param('allow_scan')?1:0;
			$SQL->UPDATE("UPDATE hosts SET cname=?,allow_scan=? WHERE uid=".$db_cuid,$db_cname,$db_callow_scan);
	    }
	}
	return ($db_cuid,$db_cname,$db_callow_scan);
}
