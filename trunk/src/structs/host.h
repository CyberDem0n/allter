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
#ifndef HOST_H_
#define HOST_H_

#include "../db/mydb.h"

class Host {
private:
	MyDB &_db;
	unsigned int _uid;
	const char *_ip;
	char *_login;
	char *_passwd;
	time_t _updated;
	bool _allow_scan;

public:
	Host(MyDB &db, unsigned int uid, const char *ip) : _db(db), _uid(uid), _ip(ip) {
		_login = _passwd = NULL;
	}

	virtual ~Host() {
		if (NULL != _login)
			free(_login);
		if (NULL != _passwd)
			free(_passwd);
	}

	unsigned int uid() {return _uid; }
	const char *ip() { return _ip; }
	const char *login() { return _login; }
	const char *passwd() { return _passwd; }
	bool scanAllowed() {return _allow_scan; }
	bool loadFromDB();
	void updateInDB(time_t updated);
};

#endif /* HOST_H_ */
