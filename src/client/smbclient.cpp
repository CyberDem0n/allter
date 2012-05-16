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
#include <dlfcn.h>
#include <talloc.h>
#include <strings.h>
#include <iostream>
#include <algorithm>
#include <string.h>
#include <errno.h>
#include "smbclient.h"
#include "libsmb_internal.h"

const char _smb_prefix[] = "smb://";

#define TALLOC_FREE(ctx) do { talloc_free(ctx); ctx=NULL; } while(0)

// Pointers to internal methods from libsmbclient
TALLOC_CTX *(*talloc_stackframe)(void);

SMBCSRV *(*SMBC_server)(TALLOC_CTX *ctx, SMBCCTX *context,
		bool connect_if_not_found, const char *server, const char *share,
		char **pp_workgroup, char **pp_username, char **pp_password);

bool (*cli_resolve_path)(TALLOC_CTX *ctx, const char *mountpt,
		const struct user_auth_info *dfs_auth_info, struct cli_state *rootcli,
		const char *path, struct cli_state **targetcli, char **pp_targetpath);

NTSTATUS (*cli_list)(struct cli_state *cli, const char *mask, uint16 attribute,
		NTSTATUS(*fn)(const char *, struct file_info *, const char *, void *),
		void *state);

int (*SMBC_errno)(SMBCCTX *context, struct cli_state *c);

bool (*cli_is_error)(struct cli_state *cli);

bool SmbClient::initialize(void) {
	static int initialized = 0;
	void *handle = RTLD_NEXT;

	if (initialized)
		return initialized==1;

	initialized = -1;

	if (NULL == (*(void **) &talloc_stackframe = dlsym(handle, "talloc_stackframe")))
		return false;
	if (NULL == (*(void **) &SMBC_server = dlsym(handle, "SMBC_server")))
		return false;
	if (NULL == (*(void **) &cli_resolve_path = dlsym(handle, "cli_resolve_path")))
		return false;
	if (NULL == (*(void **) &cli_list = dlsym(handle, "cli_list")))
		return false;
	if (NULL == (*(void **) &SMBC_errno = dlsym(handle, "SMBC_errno")))
		return false;
	if (NULL == (*(void **) &cli_is_error = dlsym(handle, "cli_is_error")))
		return false;

	initialized = 1;
	return true;
}

SmbClient::SmbClient() : _ctx(NULL), _srv(NULL) {
	setDebugLevel(0);
	setWorkGroup("WORKGROUP");
	setUser("GUEST");
	setPassword("");

	if (!initialize()) {
		getDirListFn = &SmbClient::ordinaryDirList;
		std::cerr << "Warning: Unable initialize internal libsmbclient function pointers" << std::endl;
	} else getDirListFn = &SmbClient::fastDirList;
	createContext();
}

SmbClient::~SmbClient() {
	if (_ctx) {
		smbc_getFunctionPurgeCachedServers(_ctx)(_ctx);
		smbc_free_context(_ctx, 1);
		_ctx = NULL;
	}
}

void SmbClient::auth_fn(const char *server, const char *share, char *wrkgrp,
		int wrkgrplen, char *user, int userlen, char *passwd, int passwdlen) {
	strncpy(wrkgrp, _workgroup, wrkgrplen - 1);
	wrkgrp[wrkgrplen - 1] = 0;
	strncpy(user, _username, userlen - 1);
	user[userlen - 1] = 0;
	strncpy(passwd, _password, passwdlen - 1);
	passwd[passwdlen - 1] = 0;
}

SmbClient *ths = NULL;

void smbc_auth_fn(const char *server, const char *share, char *wrkgrp,
		int wrkgrplen, char *user, int userlen, char *passwd, int passwdlen) {
	if (ths)
		ths->auth_fn(server, share, wrkgrp, wrkgrplen, user, userlen, passwd, passwdlen);
}

void SmbClient::createContext(void) throw(std::string){
	if ((_ctx = smbc_new_context()) == NULL)
		goto throw_error;

	smbc_setDebug(_ctx, _debuglevel);
	ths = this; // TODO: Not thread safe :( Put *this into hash, with pid as key
	smbc_setFunctionAuthData(_ctx, smbc_auth_fn);

	if (smbc_init_context(_ctx) == NULL) {
		smbc_free_context(_ctx, 1);
		goto throw_error;
	}

	return;
throw_error:
	throw std::string(strerror(errno));
}

void SmbClient::rebuildSmbPath(void) {
	size_t offset = sizeof(_smb_prefix) - 1;

	strcpy(_smb_path, _smb_prefix);
	strcpy(_smb_path + offset, _host);
	offset += strlen(_host);

	if (_share[0] != '\0') {
		_smb_path[offset++] = '/';
		strcpy(_smb_path + offset, _share);
		offset += strlen(_share);
		_path = _smb_path + offset;
		_path[0] = '/';	_path[1] = '\0';
	}

	if (_ctx && _srv) {
		smbc_getFunctionRemoveUnusedServer(_ctx)(_ctx, _srv);
		smbc_getFunctionRemoveCachedServer(_ctx)(_ctx, _srv);
		_srv = NULL;
	}
}

void SmbClient::setDebugLevel(int debuglevel) {
	_debuglevel = debuglevel;
}

void SmbClient::setWorkGroup(const char *workgroup) {
	strncpy(_workgroup, workgroup, sizeof(_workgroup)-1);
}

void SmbClient::setShare(const char *share) {
	IClient::setShare(share);
	rebuildSmbPath();
}

void SmbClient::setHost(const char *host) {
	IClient::setHost(host);
	rebuildSmbPath();
}

void SmbClient::dirListFn(const char *name, DIRENT_TYPE type, uint64_t size, time_t mtime) {
	struct my_dirent item = {type, size, mtime, name};
	_dir_list.push_back(item);
}

static NTSTATUS dir_list_fn(const char *mnt, struct file_info *finfo, const char *mask, void *state) {
	SmbClient *ths = (SmbClient *)state;
	DIRENT_TYPE type = (finfo->mode&FILE_ATTRIBUTE_DIRECTORY)?TYPE_DIRECTORY:TYPE_COMMON;
	ths->dirListFn(finfo->name, type, finfo->size, finfo->mtime_ts.tv_sec);
	return NT_STATUS_OK;
}

bool SmbClient::fastDirList(const char *dir) {
	int dirlen = 0;
	char netpath[2048], *targetpath;
	struct cli_state *targetcli;
	NTSTATUS status;
	TALLOC_CTX *frame = talloc_stackframe();
	char *workgroup = talloc_strdup(frame, _workgroup);
	char *user = talloc_strdup(frame, _username);
	char *password = talloc_strdup(frame, _password);

	if (NULL == _srv) {
		if (NULL == (_srv = SMBC_server(frame, _ctx, true, _host, _share, &workgroup, &user, &password))) {
			TALLOC_FREE(frame);
			return false;
		}
	}

	while (*dir) {
		netpath[dirlen++] = *dir=='/'?'\\':*dir;
		dir++;
	}

	if (netpath[dirlen-1] != '\\')
		netpath[dirlen++] = '\\';
	netpath[dirlen++] = '*';
	netpath[dirlen] = '\0';

	if (!cli_resolve_path(frame, "", _ctx->internal->auth_info,
				_srv->cli, netpath,
				&targetcli, &targetpath)) {
		std::cerr << "Could not resolve " << _host << std::endl;
		TALLOC_FREE(frame);
		return false;
	}

	status = cli_list(targetcli, targetpath,
			FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN,
			dir_list_fn, (void *)this);

	if (!NT_STATUS_IS_OK(status)) {
		errno = SMBC_errno(_ctx, targetcli);
		if (cli_is_error(targetcli) &&
				smbc_getFunctionCheckServer(_ctx)(_ctx, _srv) &&
				smbc_getFunctionRemoveUnusedServer(_ctx)(_ctx, _srv)) {
				smbc_getFunctionRemoveCachedServer(_ctx)(_ctx, _srv);
		}
		TALLOC_FREE(frame);
		return false;
	}

	TALLOC_FREE(frame);
	return true;
}

bool SmbClient::ordinaryDirList(const char *dir) {
	strncpy(_path, dir, sizeof(_smb_path) - (_path - _smb_path));
	return true;
}

std::vector<struct my_dirent> SmbClient::getDirList(const char *dir) throw (std::string) {
	checkShare();

	_dir_list.clear();
	if (!(*this.*getDirListFn)(dir))
		throw std::string(strerror(errno));
	return _dir_list;
}

std::vector<std::string> SmbClient::getShares(void) throw (std::string) {
	checkHost();

	std::vector<std::string> ret;
	struct smbc_dirent *dirent;
	SMBCFILE *fd;

	if ((fd = smbc_getFunctionOpendir(_ctx)(_ctx, _smb_path)) == NULL)
		throw std::string(strerror(errno));

	while ((dirent = smbc_getFunctionReaddir(_ctx)(_ctx, fd)) != NULL) {
		if (dirent->smbc_type != SMBC_FILE_SHARE)
			continue;
		ret.push_back(dirent->name);
	}
	smbc_getFunctionClosedir(_ctx)(_ctx, fd);
	smbc_getFunctionPurgeCachedServers(_ctx)(_ctx);
	return ret;
}
