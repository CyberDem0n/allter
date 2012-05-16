CREATE TABLE `hosts` (
  `uid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `cip` varchar(255) NOT NULL,
  `cname` varchar(255) NOT NULL,
  `updated` int(10) unsigned NOT NULL DEFAULT '0',
  `allow_scan` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `workgroup` varchar(255) NOT NULL DEFAULT '',
  `login` varchar(255) NOT NULL DEFAULT 'GUEST',
  `passwd` varchar(255) NOT NULL DEFAULT '',
  `dnsname` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `shares` (
  `uid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL,
  `host` int(10) unsigned NOT NULL,
  `updated` int(10) unsigned DEFAULT NULL,
  `update_interval` int(10) unsigned NOT NULL DEFAULT '21600',
  `allow_scan` tinyint(3) unsigned DEFAULT '1',
  `pres` int(10) unsigned NOT NULL,
  `size` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`uid`),
  KEY `shares_comp` (`host`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `paths` (
  `uid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(1024) NOT NULL,
  `share` int(10) unsigned NOT NULL,
  `p2f` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`uid`),
  UNIQUE KEY `paths_p2f_share` (`p2f`,`share`),
  KEY `paths_share_name` (`share`,`name`(255))
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `paths2files` (
  `uid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `path` int(10) unsigned NOT NULL,
  `file` int(10) unsigned NOT NULL,
  `mtime` int(10) unsigned NOT NULL,
  `updated` int(10) unsigned NOT NULL,
  `size` bigint(20) unsigned NOT NULL DEFAULT '0',
  `add_info` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`uid`),
  KEY `paths2files_path` (`path`),
  KEY `paths2files_file` (`file`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `files` (
  `uid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(1024) NOT NULL,
  `type` tinyint(3) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`uid`),
  KEY `files_name_type` (`name`(255),`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
