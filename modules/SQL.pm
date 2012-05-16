package SQL;
use strict;
use DBI;
use Exporter;
use vars qw/$VERSION/;
$VERSION=1.0;
use Data::Dumper;

sub new {
	die "Constructor SQL class without params.\n" unless @_ == 1;
	my $class=shift;
	local %_=@_;
	my $this={};
	$this->{_base}=DBI->connect('DBI:mysql:database=allter','root','') || die $DBI::errstr;
	$this->{_base}->do("SET NAMES koi8r");
	bless($this , $class);
	return $this;
}

sub dbi {
	my $this = shift || die "Call class methods without data.\n";
	return $this->{_base};
}

sub sOneBase {
	my $this = shift || die "Call class methods without data.\n";
	local %_=@_;
	$_{table} || return undef;
#	print STDERR "SELECT ".($_{fields} ? join " , ",@{$_{fields}} : '*')." FROM ".$_{table}." ".
#					($_{where} ? " WHERE ".$_{where}." ":"").
#					($_{order} ? " ORDER BY " : '').($_{order} ?  join ' , ',@{$_{order}}:'').
#					($_{limit} ? " LIMIT ".($_{from}||'0').",".$_{limit}:"");
	my $sth=$this->{_base}->prepare("SELECT ".($_{fields} ? join " , ",@{$_{fields}} : '*')." FROM ".$_{table}." ".
					($_{where} ? " WHERE ".$_{where}." ":"").
					($_{order} ? " ORDER BY " : '').($_{order} ?  join ' , ',@{$_{order}}:'').
					($_{limit} ? " LIMIT ".($_{from}||'0').",".$_{limit}:""));
	$sth->execute || die $sth->errstr;
	my $ref = $sth->fetchall_arrayref;
	$sth->finish; return $ref;
}

sub QUERY {
	my ($this,$Query,@exec_binds) = @_;
	my $sth = $this->{_base}->prepare($Query);
	$sth->execute(@exec_binds) || die $sth->errstr;
	my $ref = $sth->fetchall_arrayref;
	$sth->finish; return $ref;
}

sub UPDATE {
	my ($this,$Query,@exec_binds) = @_;
	my $sth = $this->{_base}->do($Query, undef, @exec_binds);
#	$sth->execute(@exec_binds) || die $sth->errstr;
#	return $sth->fetchall_arrayref;
}

sub PREPARE {
	my ($this, $Query) = @_;
	my $sth = $this->{_base}->prepare($Query);
	return $sth;
}

sub DO {
	my ($this,$Query,@exec_binds) = @_;
	return $this->{_base}->do($Query,undef,@exec_binds);
}

sub sDistinctChars {
	my $this = shift;
	my $sth = $this->{_base}->prepare("SELECT DISTINCT LEFT(host.cname,1) FROM hosts,shares,paths WHERE hosts.uid=host and shares.uid=share and p2f=0 ORDER BY 1");
	$sth->execute || die $sth->errstr;
	return $sth->fetchall_arrayref;
}

sub sComps {
	my ($this,$fc) = @_;
	my $sth = $this->{_base}->prepare("SELECT hosts.uid,cname,cip FROM hosts LEFT JOIN shares ON hosts.uid=host LEFT JOIN paths ON shares.uid=share WHERE LEFT(cname,1) = '$fc' AND hosts.uid=host AND shares.uid=share AND p2f=0 GROUP BY hosts.uid ORDER BY cname");
	$sth->execute || die $sth->errstr;
	return $sth->fetchall_arrayref;
}

sub sShares {
	my ($this,$comp) = @_;
	my $sth = $this->{_base}->prepare("SELECT DISTINCT(shares.uid),shares.name FROM shares,paths WHERE host=$comp AND shares.uid=share AND p2f=0 ORDER BY 2");
	$sth->execute || die $sth->errstr;
	return $sth->fetchall_arrayref;
}

sub sContent {
	my $this = shift;
	local %_ = @_;
	$_{dir} = 0 if ($_{dir} !~ /^\d+$/);
	$_{share_uid} = 0 if ($_{share_uid} !~ /^\d+$/);
	return $this->{_base}->selectall_arrayref("SELECT paths2files.uid,files.name,type,size,mtime,paths2files.updated,add_info&16383 FROM paths,paths2files,files WHERE paths.p2f=".$_{dir}." AND share=".$_{share_uid}." AND paths.uid=path AND file=files.uid ORDER BY files.name");
}

sub sParentDir {
	my $this = shift;
	local %_ = @_;
	my $sth = $this->{_base}->prepare("SELECT paths2files.uid FROM paths,paths2files,files WHERE share=$_{share} AND paths.name = ? AND paths.uid = paths2files.path AND paths2files.file = files.uid AND files.name = ?");
	$sth->execute($_{dir},$_{item}) || die $sth->errstr;
	return $sth->fetchall_arrayref->[0][0];
}

sub insert_comp {
	my $this = shift;
	local %_ = @_;
	my $sth = $this->{_base}->prepare("INSERT INTO hosts(cname,dnsname,cip,updated,allow_scan,workgroup) VALUES(?,?,?,?,?,?)");
	$sth->execute($_{cname},$_{dnsname}||$_{cname},$_{cip},time,(exists $_{allow_scan}?$_{allow_scan}:1),$_{workgroup});
	return $this->{_base}->{mysql_insertid};
}

sub insert_share {
	my $this = shift;
	local %_ = @_;
	my $sth = $this->{_base}->prepare("INSERT INTO shares(name,host,updated,update_interval,allow_scan,pres) VALUES(?,?,?,?,?,?)");
	$sth->execute($_{name},$_{parent},time-$_{update_interval},$_{update_interval},1,time-$_{update_interval});
	return $this->{_base}->{mysql_insertid};
}

sub DESTROY {
	my $this = shift;
	$this->{_base}->disconnect || die $DBI::errstr;
}

1;
