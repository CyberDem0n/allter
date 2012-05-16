#!/usr/bin/perl -w
use strict;
use Data::Dumper;

&main;

exit;

sub main {
#	chdir("/home/allter/index");
	my $last_uid = 0;
	my $size = 0;
	if(-e 'fls.txt') {
		(undef,undef,undef,undef,undef,undef,undef,$size) =  stat("fls.txt");
		$last_uid = (split(",",`/usr/bin/tail -n 1 fls.txt`))[0] if $size > 0;
	}
	$size = 0 unless(-e "tmp.bin");
	system("/usr/bin/mysql --default-character-set=koi8r -u root allter -e 'SELECT uid,type,ucase(name) FROM files WHERE uid>$last_uid ORDER BY uid' | egrep -v '^uid' | sed 's/\\t/,/g' >> fls.txt");
	$last_uid = (split(",",`/usr/bin/tail -n 1 fls.txt`))[0];
	open F,">>last_uid.txt";
	print F $last_uid." ".localtime()."\n";
	close F;
	system("nice -n 19 ./indexer fls.txt ".$size);
	rename("fls.txt.id2t","fls.txt.id2");
	rename("fls.txt.id4t","fls.txt.id4");
	rename("fls.txt.idxt","fls.txt.idx");
}
