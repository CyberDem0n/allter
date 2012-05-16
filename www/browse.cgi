#!/usr/bin/perl -w
use strict;
use locale;
use POSIX;
use HTML::Template;
use Data::Dumper;
use CGI::Fast qw(:all);
use vars qw/$script_name $ADMIN/;

BEGIN {
	unshift @INC, '/home/allter/modules';
	setlocale(LC_ALL,"ru_RU.KOI8-R");
#	$ENV{SCRIPT_FILENAME} =~ /.+\/(.+)$/;
	$script_name = 'browse.cgi';
	$ADMIN = 0;
};
use SQL;
use Common;

my $TREE = {};
my $SHARES = {};
my $COMPS = {};
my $tree_done = time;
my $tree_timeout = 300;

my $template = HTML::Template->new(filename => 'tmpls/samba.html');
my $sql = new SQL;

make_tree();

while (my $cgi = new CGI::Fast) {
	$ADMIN = 1 if($ENV{REMOTE_ADDR} eq &ADMIN_ADDRESS);

	&main($template,$cgi,$sql);
	if (time-$tree_done>$tree_timeout) {
		make_tree();
		$template = HTML::Template->new(filename => 'tmpls/samba.html');
		$tree_done = time;
	}
}

exit;

sub make_tree {
# Clean old data
	delete $SHARES->{$_} foreach (keys %$SHARES);
	delete $COMPS->{$_} foreach (keys %$COMPS);
	foreach my $fc (keys %$TREE) {
		my $cc = $TREE->{$fc};
		foreach my $c(@$cc) {
			my $ss = delete $c->{shares};
			foreach my $s(@$ss) {
				delete $s->{share_name};
				delete $s->{share_uid};
			}
			splice @$ss;
			delete $c->{comp_uid};
			delete $c->{comp_name};
			delete $c->{comp_ip};
		}
		splice @$cc;
		delete $TREE->{$fc};
	}

	my $ref = $sql->dbi->selectall_arrayref(
				q{SELECT hosts.uid,cip,cname,shares.uid,shares.name
					FROM hosts,shares,paths
					WHERE hosts.uid=host
						AND shares.uid=share
						AND paths.p2f=0
					GROUP BY 1,4
					ORDER BY 3,2,5}
			);

	foreach (@$ref) {
		my $fc = uc substr $_->[2], 0, 1;
		my ($comps, $shares);
		if ($comps = $TREE->{$fc}) {
			goto new_comp if ($comps->[$#$comps]->{comp_ip} ne $_->[1]);
		} else {
new_comp:
			push @{$TREE->{$fc}}, {comp_uid=>$_->[0], comp_ip=>$_->[1],
						comp_name=>uc($_->[2])};
			$comps = $TREE->{$fc};
			$COMPS->{$_->[0]} = $comps->[$#$comps];
		}
		push @{$comps->[$#$comps]->{shares}}, {share_uid=>$_->[3], share_name=>$_->[4]};
		$SHARES->{$_->[3]} = $comps->[$#$comps];
		splice @$_;
	}
	splice @$ref;
}

sub main {
	my ($Template,$Q,$SQL) = @_;
	print "Content-type: text/html; charset=koi8-r\n\n";
	my ($char,$comp,$share);
	foreach($Q->param) {
		if(/^ods_(\d+)$/) {
			if(!$Q->param('ds_'.$1) && $Q->param($_)) {
				$SQL->UPDATE("UPDATE paths2files SET add_info=0 WHERE uid=".$1);
			}
		}
		if(/^ds_(\d+)$/) {
			if(!$Q->param('ods_'.$1) && $Q->param($_)) {
				$SQL->UPDATE("UPDATE paths2files SET add_info=1 WHERE uid=".$1);
			}
		}
	}

	if(defined $Q->param('fc')) {
		$char = &print_chars($Template,$Q->param('fc'));
		($char,$comp) = &print_comps($Template,'',$char);
		($comp,$share) = &print_shares($Template,'',$comp);
		$share = &print_content($Template,'',$share);
		$Template->param(sel_name=>'fc',sel_value=>$char);
	}
	elsif($Q->param('comp')) {
		($char,$comp) = &print_comps($Template,$Q->param('comp'),'');
		$char = &print_chars($Template,$char);
		($comp,$share) = &print_shares($Template,'',$comp);
		$share = &print_content($Template,'',$share);
		$Template->param(sel_name=>'comp',sel_value=>$comp);
	}
	elsif($Q->param('share')) {
		($comp,$share) = &print_shares($Template,$Q->param('share'),'');
		($char,$comp) = &print_comps($Template,$comp,'');
		$char = &print_chars($Template,$char);
		$share = &print_content($Template,'',$share);
		$Template->param(sel_name=>'share',sel_value=>$share);
	}
	elsif($Q->param('dir')) {
		$share = &print_content($Template,$Q->param('dir'),'');
		($comp,$share) = &print_shares($Template,$share,'');
		($char,$comp) = &print_comps($Template,$comp,'');
		$char = &print_chars($Template,$char);
		$Template->param(sel_name=>'dir',sel_value=>$Q->param('dir'));
	}
	else {
		$char = &print_chars($Template,'A');
		($char,$comp) = &print_comps($Template,'',$char);
		($comp,$share) = &print_shares($Template,'',$comp);
		$share = &print_content($Template,'',$share);
		$Template->param(sel_name=>'fc',sel_value=>$char);
	}
	$Template->param(script_name=>$script_name,admin=>$ADMIN);
	print $Template->output;
	$Template->clear_params();
}

sub print_chars {
	my ($Template,$char) = @_;
	my ($found,@char_loop) = (0);
	foreach (sort keys %$TREE) {
		push @char_loop, {
			script_name=>$script_name,
			char=>uc($_),
			selected=>(uc($_) eq uc($char)?$found=1:0)
		};
	}
	unless ($found) {
		$char_loop[0]{selected} = 1;
		$char = $char_loop[0]{char};
	}
	$Template->param(char_loop=>\@char_loop);
	return $char;
}

sub print_comps {
	my ($Template,$comp,$char) = @_;
	if ($comp) {
		my $ref = $COMPS->{$comp};
		return unless $ref;
		$char = uc substr $ref->{comp_name}, 0, 1;
	}
	my $ref = $TREE->{$char};
	return unless $ref;
	my ($found,@comp_loop) = (0);
	foreach (@$ref) {
		push @comp_loop, {
			comp_uid => $_->{comp_uid},
			comp_name => $_->{comp_name},
			comp_ip => $_->{comp_ip},
			script_name => $script_name,
			selected => ($_->{comp_uid} eq $comp?($found = 1+scalar(@comp_loop)):0)
		};
	}
	if ($found>0) { $found--; }
	else { $comp_loop[$found]{selected} = 1; }
#	$ADMIN = 1 if($comp_loop[$found]{comp_ip} eq $ENV{REMOTE_ADDR}); # Uncomment if your want allow users manage his directories
	$Template->param(comp=>$comp_loop[$found]{comp_name},ip=>$comp_loop[$found]{comp_ip},&get_status($comp_loop[$found]{comp_ip})=>1);
	$Template->param(comp_loop=>\@comp_loop);
	return (substr ($comp_loop[$found]{comp_name}, 0, 1), $comp_loop[$found]{comp_uid});
}

sub print_shares {
	my ($Template,$share,$comp) = @_;
	my $ref = $share?$SHARES->{$share}:$COMPS->{$comp};
	return unless $ref;
	$comp = $ref->{comp_uid} if ($share);
	my ($found, @share_loop) = (0);
	foreach (@{$ref->{shares}}) {
		push @share_loop, {
			share_uid => $_->{share_uid},
			share_name => $_->{share_name},
			selected => ($_->{share_uid} eq $share?($found = $_->{share_name}):0)
		};
	}
	if ($found) {
		$Template->param(share=>$found);
	} else {
		$share_loop[0]{selected} = 1;
		$Template->param(share=>$share_loop[0]{share_name});
	}
	$Template->param(share_loop=>\@share_loop);
	return ($comp,$share_loop[0]{share_uid}) unless $found;
	return ($comp,$share);
}

sub print_content {
	my ($Template,$dir,$share) = @_;
	my $up = 0;
	if($dir) {
		$dir = 0 if ($dir !~ /^\d+$/);
		my $temp = $sql->dbi->selectall_arrayref(
				q{SELECT share,paths.name,files.name,p2f
					FROM paths,paths2files,files
					WHERE paths.uid=paths2files.path
						AND paths2files.file=files.uid
						AND paths2files.uid=}.$dir
				);
		$share = $temp->[0][0];
		$Template->param(path=>$temp->[0][1].$temp->[0][2]."\\");
		if ($temp->[0][3]) {
			$Template->param(show=>'dir',up=>$temp->[0][3]);
		} else {
			$Template->param(show=>'share',up=>$share);
		}
		$up = 1;
	}
	else {
		$dir = 0;
		$Template->param(path=>"\\");
	}
	$share = 0 if ($share !~ /^\d+$/);
	my $cont = $sql->dbi->selectall_arrayref(
				q{SELECT paths2files.uid,files.name,type,size,mtime,paths2files.updated,add_info&16383
					FROM paths,paths2files,files
					WHERE paths.p2f=}.$dir.q{
						AND share=}.$share.q{
						AND paths.uid=path
						AND file=files.uid
					ORDER BY files.name}
			);
	my ($ad,$af,@cld,@clf) = (1,1);
	foreach(@$cont) {
		if($_->[2] eq '2') {
			my $chbox = 0;
			$chbox = 1 if($ADMIN);
			push @cld,{script_name=>$script_name,chbox=>$chbox,checked=>$_->[6],content_uid=>$_->[0],content_name=>$_->[1],
			dir=>1,size=>'0',mtime=>&utime2str($_->[4]),added=>&utime2str($_->[5]),bg=>((($ad++)+$up)%2)};
		} else {
			push @clf,{script_name=>$script_name,content_uid=>$_->[0],content_name=>$_->[1].($_->[2]==3?" ".$_->[6]:""),
			dir=>0,size=>&printd($_->[3]),mtime=>&utime2str($_->[4]),added=>&utime2str($_->[5]),bg=>((($af++)+$up)%2)};
		}
		splice @$_;
	}
	splice @$cont;
	if($#cld!=-1 && $#clf!=-1 && $cld[$#cld]{bg} eq $clf[0]{bg}) {
		$_->{bg} = ($_->{bg}+1)%2 foreach(@clf);
	}
	$Template->param(content_loop=>[@cld,@clf]);
	make_tree() unless $SHARES->{$share};
	return $share;
}

