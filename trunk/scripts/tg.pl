#!/usr/bin/perl
use strict;
use Data::Dumper;
use HTML::Template;
use locale;

BEGIN {
	push @INC,'/home/allter/modules';
}
use SQL;
use Common;

my $timestamp = time;
my $SQL = new SQL;

my $comps = $SQL->QUERY("SELECT COUNT(DISTINCT hosts.uid) FROM hosts, shares, paths WHERE hosts.uid=host AND shares.uid=share AND paths.p2f=0")->[0][0];

my $shares = $SQL->QUERY("SELECT COUNT(*) FROM paths WHERE p2f=0")->[0][0];

my $files = $SQL->QUERY("SELECT COUNT(*) FROM paths2files")->[0][0];

my $mp3s = $SQL->QUERY("SELECT COUNT(*) FROM mp3_info")->[0][0];

my $ssize = $SQL->QUERY("SELECT SUM(size) FROM paths2files")->[0][0];

my $Template = HTML::Template->new(filename => &WWWROOT.'tmpls/stat_table.tmpl');
$Template->param(comps=>&printd($comps),shares=>&printd($shares),total_files=>&printd($files),mp3=>&printd($mp3s),total_size=>&printd($ssize));
open FILE,">".&WWWROOT."tmpls/stat_table.html";
print FILE $Template->output;
close FILE;

exit;
