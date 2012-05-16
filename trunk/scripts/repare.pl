#!/usr/bin/perl
use strict;
use POSIX;
use Data::Dumper;
use locale;
BEGIN {
	push @INC,'/home/allter/modules';
}
use SQL;

$| = 1;
my $block_size = 1000;
my $timestamp = time;
my $SQL = new SQL;

print "������� ������ �� shares, ������� ��������� �� �������������� ������ � ������� hosts: ";
print 0+$SQL->DO("DELETE shares FROM shares LEFT JOIN hosts ON host = hosts.uid WHERE hosts.uid IS NULL"),"\n"; # ������� ������ �� shares, ������� ��������� �� �������������� ������ � ������� hosts

print "������� ������ �� paths, ������� ��������� �� �������������� ������ � ������� shares: ";
print 0+$SQL->DO("DELETE paths FROM paths LEFT JOIN shares ON share = shares.uid WHERE shares.uid IS NULL"),"\n"; # ������� ������ �� paths, ������� ��������� �� �������������� ������ � ������� shares

print "������� ������ �� ������� paths2files, ������� ��������� �� �������������� ������ � ������� paths: ";
print 0+$SQL->DO("DELETE paths2files FROM paths2files LEFT JOIN paths ON paths.uid = paths2files.path WHERE paths.uid IS NULL"),"\n"; # ������� ������ �� ������� paths2files, ������� ��������� �� �������������� ������ � ������� paths

print "������� ������ �� ������� paths2files, ������� ��������� �� �������������� ������ � ������� files: ";
print 0+$SQL->DO("DELETE paths2files FROM paths2files LEFT JOIN files ON files.uid = paths2files.file WHERE files.uid IS NULL"),"\n"; # ������� ������ �� ������� paths2files, ������� ��������� �� �������������� ������ � ������� files

print "������� ������ � ������� paths � p2f==0, � name!=\"\\\" (BIG ERROR)\n";
&do_delete_paths($SQL->QUERY("SELECT paths.name,share FROM paths WHERE p2f=0 AND paths.name!='\\\\'")); # ������� ������ � ������� paths � p2f==0, � name=="\\" (BIG ERROR)


print "������� ������ � ������� paths, ������� ��������� �� �������������� ������ � ������� paths2files\n";
&do_delete_paths($SQL->QUERY("SELECT paths.name,share FROM paths LEFT JOIN paths2files ON paths.p2f = paths2files.uid WHERE paths2files.uid IS NULL AND paths.p2f != 0")); # ������� ������ � ������� paths, ������� ��������� �� �������������� ������ � ������� paths2files


print "������� ������ � ������� paths, ������� ��������� �����������\n";
&do_delete_paths($SQL->QUERY("SELECT paths.name,share FROM paths2files,paths WHERE add_info = 1 AND paths2files.uid = paths.p2f")); # ������� ������ � ������� paths, ������� ��������� �����������

#print "������� ������ �� mp3_info, ������� ��������� �� �������������� ������ � ������� paths2files: ";
#print 0+$SQL->DO("DELETE mp3_info FROM mp3_info LEFT JOIN paths2files ON mp3_info.parent = paths2files.uid WHERE paths2files.uid IS NULL"),"\n"; # ������� id ������� �� mp3_info, ������� ��������� �� �������������� ������ � ������� paths2files

print "������� ������ �� files, �� ������� ��� ������ �� ������� paths2files: ";
print 0+$SQL->DO("DELETE files FROM files LEFT JOIN paths2files ON files.uid = paths2files.file WHERE paths2files.file IS NULL"),"\n"; # ������� ������ �� files, �� ������� ��� ������ �� ������� paths2files

print "����� ��������� �������: ".(time-$timestamp)." ������\n";
exit;


sub do_delete_paths {
	my $sth_ref = shift;
	foreach(@{$sth_ref}) {
		print 0+$SQL->DO("DELETE p2f FROM paths p,paths2files p2f WHERE share=".$_->[1]." AND p.name like ? AND p2f.path=p.uid",&escape($_->[0])."%"),"\n";
		print 0+$SQL->DO("DELETE FROM paths WHERE share=".$_->[1]." AND name LIKE ?",&escape($_->[0])."%"),"\n";
    }
}

sub escape {
	my $str = shift;
	$str =~ s/(\\|\')/\\$1/g;
	return $str;
}
