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

#include "share.h"

const char *Share::escaped_name() {
	if (NULL == _escaped_name)
		_escaped_name = _db.escape(_name.c_str());
	return _escaped_name;
}

bool Share::loadFromDB() {
	MYSQL_RES *res;

	if (_db.query("SELECT uid,updated,update_interval,pres,allow_scan,size FROM shares WHERE host=%u AND name='%s'", _hostId, escaped_name())
			&& (res = _db.results())) {
		MYSQL_ROW row = mysql_fetch_row(res);
		if (row) {
			_uid = atoi(row[0]);
			_updated = atoi(row[1]);
			_update_interval = atoi(row[2]);
			_presented = atoi(row[3]);
			_allow_scan = atoi(row[4]);
			_size = atol(row[5]);
		}
		mysql_free_result(res);
	}

	if (_uid < 1)
		return insertIntoDB();
	return true;
}

bool Share::insertIntoDB() {
	_uid = _db.insert("INSERT INTO shares(name,host,updated,update_interval,pres,allow_scan,size) VALUES('%s',%u,%u,%u,%u,%d,%llu)", escaped_name(), _hostId, _updated, _update_interval, _presented, _allow_scan, _size);
	return (_uid > 0);
}

void Share::updateInDB(time_t presented) {
	if (_presented != presented)
		_db.query("UPDATE shares SET pres=%u WHERE uid=%u", (_presented=presented), _uid);
}

void Share::updateInDB(time_t presented, time_t updated, uint64_t size) {
	if (_presented != presented || _updated != updated || _size != size)
		_db.query("UPDATE shares SET pres=%u, updated=%u, size=%llu WHERE uid=%u", (_presented=presented), (_updated = updated), (_size=size), _uid);
}
