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
#include <wctype.h>

#include "dirent.h"

Dirent::Dirent(MyDB &db, struct my_dirent &item) : _db(db), _escaped_name(NULL), _upper_name(NULL) {
	_name = item.name;
	_size = item.size;
	_mtime = item.mtime;
	_add_info = _uid = _path = _file = 0;
	if ((_namelen = strlen(_name.c_str()))>0 && NULL != (_upper_name = upname()))
		_type = item.type==TYPE_DIRECTORY?TYPE_DIRECTORY:getFileType();
}

Dirent::~Dirent() {
	if (NULL != _escaped_name)
		delete [] _escaped_name;
	if (NULL != _upper_name)
		delete [] _upper_name;
}

bool Dirent::strequal(const char *s1, const char *s2) {
	return (strcasecmp(s1, s2) == 0);
}

const char *Dirent::upname() const {
	if (_upper_name != NULL)
		return _upper_name;

	const char *str = _name.c_str();
	size_t len = _namelen + 1;

	if (_namelen < 1) return NULL;

	wchar_t *it, *tmp = new wchar_t[len];
	char *ret;

	if ((it = tmp) == NULL) return NULL;

	if ((mbstowcs(tmp, str, len) < 1) ||
			(ret = new char[len]) == NULL) {
		delete [] tmp;
		return NULL;
	}

	while (*it) {
		*it = towupper(*it);
		it++;
	}

	if ((wcstombs(ret, tmp, len)) < 1) {
		delete [] ret;
		ret = NULL;
	}

	delete [] tmp;
	return ret;
}

DIRENT_TYPE Dirent::getFileType() {
	const char *name = _name.c_str();
	const char *ext;
	if (_namelen < 3)
		return TYPE_COMMON;

	ext = name + _namelen - 3;
	if (strequal(ext, ".ra"))
		return TYPE_AUDIO;
	if (strequal(ext, ".ts") || strequal(ext, ".rm"))
		return TYPE_VIDEO;

	if (_namelen < 4)
		return TYPE_COMMON;

	ext = name + _namelen - 4;
	if (strequal(ext, ".mp3") || strequal(ext, ".ogg") || strequal(ext, ".aac")
		|| strequal(ext, ".wma") || strequal(ext, ".mpc")
		|| strequal(ext, ".ape") || strequal(ext, ".wav")
		|| strequal(ext, ".shn") || strequal(ext, ".lqt"))
			return TYPE_AUDIO;
	if (strequal(ext, ".avi") || strequal(ext, ".mpg")
		|| strequal(ext, ".vob") || strequal(ext, ".mkv")
		|| strequal(ext, ".mov") || strequal(ext, ".mp4")
		|| strequal(ext, ".3gp") || strequal(ext, ".flv")
		|| strequal(ext, ".wmv") || strequal(ext, ".ogm"))
			return TYPE_VIDEO;

	if (_namelen < 5)
		return TYPE_COMMON;

	ext = name + _namelen - 5;
	if (strequal(ext, ".flac"))
		return TYPE_AUDIO;
	if (strequal(ext, ".mpeg"))
		return TYPE_VIDEO;

	return TYPE_COMMON;
}

bool Dirent::insertIntoDB(unsigned int uid) {
	MYSQL_RES *res;
	MYSQL_ROW row;
	time_t updated = time(NULL);

	if (_escaped_name == NULL)
		_escaped_name = _db.escape(_name.c_str());

	// TODO: add LOCK TABLE files, to prevent duplicates and errors
	if (_db.query("SELECT uid FROM files WHERE name='%s' AND type=%d", _escaped_name, _type)
			&& (res = _db.results())) {
		if ((row = mysql_fetch_row(res)))
			_file = atoi(row[0]);
		mysql_free_result(res);
	}

	if (_file < 1)
		_file = _db.insert("INSERT INTO files VALUES(NULL,'%s',%d)", _escaped_name, _type);

	if (_file < 1) return false;

	if (uid)
		return _db.query("UPDATE paths2files SET file=%u, mtime=%u, updated=%u, size=%llu, add_info=0 WHERE uid=%u", _file, _mtime, updated, _size, (_uid=uid));
	else _uid = _db.insert("INSERT INTO paths2files VALUES(NULL,%u,%u,%u,%u,%llu,0)", _path, _file, _mtime, updated, _size);
	return (_uid > 0);
}

void Dirent::updateInDB(unsigned int uid) {
	_db.query("UPDATE paths2files SET mtime=%u WHERE uid=%u", _mtime, (_uid = uid));
}

void Dirent::updateInDB(unsigned int uid, time_t updated) {
	_db.query("UPDATE paths2files SET mtime=%u, updated=%u, size=%llu WHERE uid=%u", _mtime, updated, _size, (_uid = uid));
}

void Dirent::forbidScan() {
	_add_info = 1;
	_db.query("UPDATE paths2files SET add_info=1, size=0 WHERE uid=%u", _uid);
}

void Dirent::updateSizeInDB(uint64_t size) {
	if (_size != size)
		_db.query("UPDATE paths2files SET size=%llu WHERE uid=%u", (_size = size), _uid);
}
