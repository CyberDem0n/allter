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
#ifndef LIBSMB_INTERNAL_H_
#define LIBSMB_INTERNAL_H_

#include <stdint.h>
#include <libsmbclient.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void TALLOC_CTX;
#define TALLOC_FREE(ctx) do { talloc_free(ctx); ctx=NULL; } while(0)
#define talloc_free(ctx) _talloc_free(ctx, __location__)
#ifndef __location__
#define __TALLOC_STRING_LINE1__(s)    #s
#define __TALLOC_STRING_LINE2__(s)   __TALLOC_STRING_LINE1__(s)
#define __TALLOC_STRING_LINE3__  __TALLOC_STRING_LINE2__(__LINE__)
#define __location__ __FILE__ ":" __TALLOC_STRING_LINE3__
#endif

#define FILE_ATTRIBUTE_HIDDEN		0x002L
#define FILE_ATTRIBUTE_SYSTEM		0x004L
#define FILE_ATTRIBUTE_DIRECTORY	0x010L

#define likely(x) (x)
#define NT_STATUS_V(x) ((x).v)
#define NT_STATUS_IS_OK(x) (likely(NT_STATUS_V(x) == 0))
#define NT_STATUS(x) ((NTSTATUS) { x })
#define NT_STATUS_OK NT_STATUS(0x0000)

typedef uint16_t uint16;
typedef struct {
	uint32_t v;
} NTSTATUS;

struct _SMBCSRV {
	struct cli_state *cli;
/*	dev_t dev;
	bool no_pathinfo;
	bool no_pathinfo2;
	bool no_nt_session;
	struct policy_handle pol;

	SMBCSRV *next, *prev;*/
};

struct SMBC_internal_data {
	bool initialized;
	struct smbc_dirent dirent;
	char _dirent_name[1024];
	SMBCSRV * servers;
	SMBCFILE * files;
	bool full_time_names;
	smbc_share_mode share_mode;
	smbc_get_auth_data_with_context_fn auth_fn_with_context;
	void * user_data;
	smbc_smb_encrypt_level smb_encryption_level;
	bool case_sensitive;
	struct user_auth_info *auth_info;
	struct smbc_server_cache * server_cache;
	struct {
		smbc_statvfs_fn statvfs_fn;
		smbc_fstatvfs_fn fstatvfs_fn;
		smbc_ftruncate_fn ftruncate_fn;
	} posix_emu;

};

struct file_info {
	uint64_t size;
	uint16 mode;
	uid_t uid;
	gid_t gid;
	/* these times are normally kept in GMT */
	struct timespec mtime_ts;
	struct timespec atime_ts;
	struct timespec ctime_ts;
	char *name;
	char short_name[13 * 3]; /* the *3 is to cope with multi-byte */
};

// Talloc workaround, because it's header file doesn't take care about c++
char *talloc_strdup(const void *t, const char *p);
int _talloc_free(void *ptr, const char *location);


 // Pointers to internal methods from libsmbclient
TALLOC_CTX *talloc_stackframe(void);

SMBCSRV *SMBC_server(TALLOC_CTX *ctx, SMBCCTX *context,
                bool connect_if_not_found, const char *server, const char *share,
                char **pp_workgroup, char **pp_username, char **pp_password);

bool cli_resolve_path(TALLOC_CTX *ctx, const char *mountpt,
                const struct user_auth_info *dfs_auth_info, struct cli_state *rootcli,
                const char *path, struct cli_state **targetcli, char **pp_targetpath);

NTSTATUS cli_list(struct cli_state *cli, const char *mask, uint16 attribute,
                NTSTATUS(*fn)(const char *, struct file_info *, const char *, void *),
                void *state);

int SMBC_errno(SMBCCTX *context, struct cli_state *c);

bool cli_is_error(struct cli_state *cli);

#ifdef __cplusplus
}
#endif

#endif /* LIBSMB_INTERNAL_H_ */
