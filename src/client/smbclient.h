/* This file is part of AllTer file Searcher
Copyright (C) 2001-2012 Alexander Kukushkin

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>
*/
#ifndef SMBCLIENT_H_
#define SMBCLIENT_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <libsmbclient.h>

#include "iclient.h"

class SmbClient : public IClient {
private:
	int _debuglevel;
	char _workgroup[255];
	char *_path;
	char _smb_path[2048];
	SMBCCTX *_ctx;
	SMBCSRV *_srv;
	std::vector<struct my_dirent> _dir_list;

	bool ordinaryDirList(const char *dir);
	bool fastDirList(const char *dir);
	void createContext(void) throw(std::string);
	bool initialize(void);
	void rebuildSmbPath(void);
	bool (SmbClient::*getDirListFn)(const char *dir);

public:
	void dirListFn(const char *name, DIRENT_TYPE type, uint64_t size, time_t mtime);
	SmbClient();
	virtual ~SmbClient();
	void auth_fn(const char *server, const char *share, char *wrkgrp,
					int wrkgrplen, char *user, int userlen, char *passwd,
					int passwdlen);
	void setDebugLevel(int debuglevel);
	void setWorkGroup(const char *workgroup);
	virtual void setHost(const char *host);
	virtual void setShare(const char *share);
	virtual std::vector<std::string> getShares(void) throw (std::string);
	virtual std::vector<struct my_dirent> getDirList(const char *dir) throw (std::string);
};

#endif /* SMBCLIENT_H_ */
