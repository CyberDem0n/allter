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
#ifndef ICLIENT_H_
#define ICLIENT_H_

#include <stdint.h>
#include <string.h>
#include <vector>

typedef enum {
	TYPE_COMMON = 1,
	TYPE_DIRECTORY,
	TYPE_AUDIO,
	TYPE_VIDEO,
	TYPE_IMAGE
} DIRENT_TYPE;

struct my_dirent {
	DIRENT_TYPE type;
	uint64_t size;
	time_t mtime;
	std::string name;
};

static bool checkFileName(const char *name) {
	return !(NULL == name || '\0' == *name || 0 == strcmp(name, ".") ||
			0 == strcmp(name, "."));
}

class IClient {
	bool (*nameFilter)(const char *name);

protected:
	char _username[255];
	char _password[255];
	char _host[255];
	char _share[255];

	void checkHost() throw(std::string) {
		if (0 == _host[0])
			throw std::string("Host is empty");
	}

	void checkShare() throw(std::string) {
		checkHost();
		if (0 == _share[0])
			throw std::string("Share is empty");
	}

	bool checkName(const char *name) {
		return nameFilter(name);
	}
public:
	IClient() {
		_host[0] = _share[0] = _password[0] = '\0';
		setUser("GUEST");
		setNameFilter(checkFileName);
	}

	virtual ~IClient() {}

	virtual void setTimeout(int timeout) {}

	virtual void setUser(const char *user) {
		strncpy(_username, user, sizeof(_username)-1);
	}

	virtual void setPassword(const char *passwd) {
		strncpy(_password, passwd, sizeof(_password)-1);
	}

	virtual void setHost(const char *host) {
		if (strcmp(host, _host)) {
			strncpy(_host, host, sizeof(_host)-1);
			_share[0] = '\0';
		}
	}

	virtual void setShare(const char *share) {
		strncpy(_share, share, sizeof(_share)-1);
	}

	virtual std::vector<std::string> getShares(void) throw (std::string) = 0;

	virtual std::vector<struct my_dirent> getDirList(const char *dir) throw (std::string) = 0;

	virtual void  setNameFilter(bool (*filter)(const char *)) {
		nameFilter = filter;
	}
};

#endif /* ICLIENT_H_ */
