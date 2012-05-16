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
#ifndef DIRENT_H_
#define DIRENT_H_

#include <string>

#include "../client/iclient.h"
#include "../db/mydb.h"

class Dirent {
private:
	MyDB &_db;

	std::string _name;
	size_t _namelen;
	const char *_escaped_name;
	const char *_upper_name;

	DIRENT_TYPE _type;
	uint64_t _size;
	time_t _mtime;
	unsigned int _add_info, _uid, _path, _file;
	DIRENT_TYPE getFileType();
	bool strequal(const char *s1, const char *s2);
public:
	Dirent(MyDB &db, struct my_dirent &item);
	virtual ~Dirent();
	bool isValid() { return (_upper_name != NULL); }
	const char *upname() const;
//	std::string &name() { return _name; }
	unsigned int uid() { return _uid; }
	void uid(unsigned int uid) { _uid = uid; }
	void path(unsigned int path) { _path = path; }
	bool scanAllowed() { return (_type == TYPE_DIRECTORY && _add_info == 0); }
	void add_info(unsigned int add_info) { _add_info = add_info; }
	const char *name() { return _name.c_str(); }
	size_t namelen() { return _namelen; }
	unsigned int type() { return _type; }
	uint64_t size() { return _size; }
	void size(uint64_t size) { _size = size; }
	time_t mtime() {return _mtime; }
	bool loadFromDB();
	bool insertIntoDB(unsigned int uid);
	void updateInDB(unsigned int uid);
	void updateInDB(unsigned int uid, time_t updated);
	void forbidScan();
	void updateSizeInDB(uint64_t size);
};

#endif /* DIRENT_H_ */
