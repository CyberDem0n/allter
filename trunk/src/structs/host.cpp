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
#include <stdlib.h>
#include <string.h>
#include <string>

#include "host.h"

bool Host::loadFromDB() {
	MYSQL_RES *res;
	std::string error_msg;
	bool ret = false;

	if (_db.query("SELECT cip, login, passwd, allow_scan FROM hosts WHERE uid=%u", _uid)
			&& (res = _db.results())) {
		MYSQL_ROW row = mysql_fetch_row(res);
		if (row) {
			if (strcmp(_ip, row[0])) {
				error_msg = "Specified ip differs from ip in database: ";
				error_msg += row[0];
			}
			_login = strdup(row[1]);
			_passwd = strdup(row[2]);
			_allow_scan = atoi(row[3]);
			ret = true;
		}
		mysql_free_result(res);
	}

	if (error_msg.length())
		throw error_msg;

	return ret;
}

void Host::updateInDB(time_t updated) {
	if (_updated != updated)
		_db.query("UPDATE hosts SET updated=%u WHERE uid=%u", (_updated=updated), _uid);
}
