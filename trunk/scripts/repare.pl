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

print "Удаляем записи из shares, которые ссылаются на несуществующие записи в таблице hosts: ";
print 0+$SQL->DO("DELETE shares FROM shares LEFT JOIN hosts ON host = hosts.uid WHERE hosts.uid IS NULL"),"\n"; # Удаляем записи из shares, которые ссылаются на несуществующие записи в таблице hosts

print "Удаляем записи из paths, которые ссылаются на несуществующие записи в таблице shares: ";
print 0+$SQL->DO("DELETE paths FROM paths LEFT JOIN shares ON share = shares.uid WHERE shares.uid IS NULL"),"\n"; # Удаляем записи из paths, которые ссылаются на несуществующие записи в таблице shares

print "Удаляем записи из таблицы paths2files, которые ссылаются на несуществующие записи в таблице paths: ";
print 0+$SQL->DO("DELETE paths2files FROM paths2files LEFT JOIN paths ON paths.uid = paths2files.path WHERE paths.uid IS NULL"),"\n"; # Удаляем записи из таблицы paths2files, которые ссылаются на несуществующие записи в таблице paths

print "Удаляем записи из таблицы paths2files, которые ссылаются на несуществующие записи в таблице files: ";
print 0+$SQL->DO("DELETE paths2files FROM paths2files LEFT JOIN files ON files.uid = paths2files.file WHERE files.uid IS NULL"),"\n"; # Удаляем записи из таблицы paths2files, которые ссылаются на несуществующие записи в таблице files

print "Находим записи в таблице paths с p2f==0, и name!=\"\\\" (BIG ERROR)\n";
&do_delete_paths($SQL->QUERY("SELECT paths.name,share FROM paths WHERE p2f=0 AND paths.name!='\\\\'")); # Находим записи в таблице paths с p2f==0, и name=="\\" (BIG ERROR)


print "Находим записи в таблице paths, которые ссылаются на несуществующие записи в таблице paths2files\n";
&do_delete_paths($SQL->QUERY("SELECT paths.name,share FROM paths LEFT JOIN paths2files ON paths.p2f = paths2files.uid WHERE paths2files.uid IS NULL AND paths.p2f != 0")); # Находим записи в таблице paths, которые ссылаются на несуществующие записи в таблице paths2files


print "Находим записи в таблице paths, которые запрещено сканировать\n";
&do_delete_paths($SQL->QUERY("SELECT paths.name,share FROM paths2files,paths WHERE add_info = 1 AND paths2files.uid = paths.p2f")); # Находим записи в таблице paths, которые запрещено сканировать

#print "Удаляем записи из mp3_info, которые ссылаются на несуществующие записи в таблице paths2files: ";
#print 0+$SQL->DO("DELETE mp3_info FROM mp3_info LEFT JOIN paths2files ON mp3_info.parent = paths2files.uid WHERE paths2files.uid IS NULL"),"\n"; # Находим id записей из mp3_info, которые ссылаются на несуществующие записи в таблице paths2files

print "Удаляем записи из files, на которые нет ссылки из таблицы paths2files: ";
print 0+$SQL->DO("DELETE files FROM files LEFT JOIN paths2files ON files.uid = paths2files.file WHERE paths2files.file IS NULL"),"\n"; # Удаляем записи из files, на которые нет ссылки из таблицы paths2files

print "Всего затрачено времени: ".(time-$timestamp)." секунд\n";
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
