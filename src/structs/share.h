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
#ifndef SHARE_H_
#define SHARE_H_

#include "../db/mydb.h"

class Share {
private:
	MyDB &_db;
	unsigned int _hostId;
	std::string &_name;
	char *_escaped_name;
	unsigned int _uid, _update_interval;
	time_t _updated, _presented;
	bool _allow_scan;
	uint64_t _size;
	const char *escaped_name();
public:
	Share(MyDB &db, unsigned int hostId, std::string &name) : _db(db), _hostId(hostId), _name(name), _escaped_name(NULL) {
		_update_interval = 21600;
		_allow_scan = true;
		_updated = _uid =_size = 0;
		_presented = time(NULL);
	}
	virtual ~Share() {
		if (_escaped_name != NULL)
			delete [] _escaped_name;
	}
	unsigned int uid() {return _uid; }
	const char *name() {return _name.c_str(); }
	bool isOutdated() {return (time(NULL) > _updated + _update_interval);}
	bool scanAllowed() {return _allow_scan; }
	bool loadFromDB();
	bool insertIntoDB();
	void updateInDB(time_t presented);
	void updateInDB(time_t presented, time_t updated, uint64_t size);
};


#endif /* SHARE_H_ */
