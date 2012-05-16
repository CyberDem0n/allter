#!/usr/bin/perl
use strict;
use locale;
use POSIX;
use Data::Dumper;

BEGIN {
	push @INC,'../modules';
	setlocale(LC_ALL,"ru_RU.KOI8-R");
}
use SQL;

my $block_size = 100;
my $tm = time;

my @folders;

&main;

exit;

sub main {
		my $i = 0;
		my $SQL = new SQL;
		
		my $uids = "";
		foreach(@{$SQL->QUERY("SELECT DISTINCT(path) FROM paths2files,files WHERE type=4 AND files.uid=file AND size>100000000".($ARGV[0]?" AND paths2files.updated>".($tm - 24*3600):""))}) {
				$uids .= $_->[0].",";
				if(++$i >= $block_size) {
						$i = 0;
						$uids =~ s/,$//g;
						&get_files($SQL,$uids);
						$uids = "";
				}
		}
		if($i) {
				$uids =~ s/,$//g;
				&get_files($SQL,$uids);
		}
		@folders = sort {uc($a->{name}) cmp uc($b->{name})} @folders;
		print "<html>
<head>
	<title>AllTer:-> Movie List</title>
	<meta http-equiv=Content-Type content=\"text/html; charset=koi8-r\">
</head>
<body bgcolor=#ffffff text=#000000>\n";
		print "Last updated: ".localtime(time)."<br>\n";
		print &print_html;
		print "</body>
</html>";
#		print Dumper @folders;
}

sub get_files($$) {
		my ($SQL,$uids) = @_;
		my $prev = "";
		my @names;
		foreach my $file (@{$SQL->QUERY("SELECT paths.uid, cip, cname, shares.name, paths.name, files.name, size FROM hosts, shares, paths, paths2files, files WHERE path IN (".$uids.")".($ARGV[0]?" AND paths2files.updated > ".($tm - 24*3600):"")." AND file=files.uid AND type=4 AND path=paths.uid AND share=shares.uid AND host=hosts.uid ORDER BY 1,6")}) {
				if($file->[0] eq $prev) {
						push @{$folders[$#folders]->{content}},{name=>$file->[5], size=>$file->[6]};
				} else {
						pop @folders,undef if(scalar @folders && $#{$folders[$#folders]->{content}} > 255);
						$prev = $file->[0];
						my $name;
						if($file->[4] eq "\\") {
								$name = $file->[3];
						} else {
								$file->[4] =~ /^.*\\(.*?)\\$/;
								$name = $1;
						}
						push @folders,{ip=>$file->[1], path=>"\\\\".$file->[2]."\\".$file->[3].$file->[4], name=>$name, content=>[{name=>$file->[5], size=>$file->[6]}]};
				}
		}
}

sub print_html {
		my $i = 0;
		my $out = "<table border=1>\n";
		$out .= "<tr><td><b>IP-адрес, расположение</b></td><td><b>Название</b></td><td><b>Размер</b></td><td colspan=2><b>Файлы</b></td></tr>";
		foreach my $d (@folders) {
				my $ssize = 0;
				my $temp = "";
				for my $i(1..$#{$d->{content}}) {
						$ssize += ${$d->{content}}[$i]->{size};
						$temp .= "<tr><td>".&print_size($d->{content}->[$i]->{size})."</td>";
						$temp .= "<td>".$d->{content}->[$i]->{name}."</td></tr>\n";
				}
				$out .= "<tr>";
				$out .= "<td rowspan=".(scalar @{$d->{content}}).">";
				$out .= $d->{ip}."<br>".$d->{path}."</td>";
				$out .= "<td rowspan=".(scalar @{$d->{content}}).">";
				$out .= $d->{name}."</td>";
				$out .= "<td rowspan=".(scalar @{$d->{content}}).">";
				$out .= &print_size($d->{content}->[0]->{size} + $ssize)."</td>";
				$out .= "<td>".&print_size($d->{content}->[0]->{size})."</td>";
				$out .= "<td>".$d->{content}->[0]->{name}."</td></tr>\n";
				$out .= $temp;
				if(++$i>=$block_size) {
						$out .= "</table><table border=1>";
						$i=0;
				}
		}
		$out .= "</table>\n";
		return $out;
}

sub print_size($) {
	my $size = shift;
	my $t = $size/1024/1024/1024;
	return sprintf("%.1f Gb",$t) if($t>1);
	$t = $size/1024/1024;
	return sprintf("%.1f Mb",$t) if($t>1);
	$t = $size/1024;
	return sprintf("%.1f Kb",$t) if($t>1);
	return $size." b";
}
