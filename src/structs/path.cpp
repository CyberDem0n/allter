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

#include "path.h"
#include "../libs/md5.h"

Path::Path(MyDB &db, unsigned int share) : _db(db), _share(share) {
	_deep = 0;
	_path[0] = '/';
	_path[1] = '\0';
	_path_info[_deep].uid = 0;
	_path_info[_deep].p2f = 0;
	_path_info[_deep].length = 1;
	createLists();
}

Path::~Path() {
	do {
		clearList();
		clearDirs();
	} while (--_deep >= 0);
}

bool Path::goDown(unsigned int p2f, const char *name) {
	size_t length = strlen(name);
	size_t oldlen = _path_info[_deep].length;
	if (_deep + 1 >= MAX_DEEP ||
			oldlen + length > MAX_PATH)
		return false;
	memcpy(_path + oldlen, name, length);
	size_t newlen = oldlen + length;
	_path[newlen++] = '/';
	_path[newlen] = '\0';
	_path_info[++_deep].uid = 0;
	_path_info[_deep].p2f = p2f;
	_path_info[_deep].length = newlen;
	createLists();
	return true;
}

bool Path::goUp() {
	if (_deep <= 0)
		return false;
	clearList();
	clearDirs();
	_deep--;
	_path[_path_info[_deep].length] = '\0';
	return true;
}

bool Path::detectSelfLooping() {
	if (_deep<2) return false;
	if (!memcmp(_path_info[_deep].md5, _path_info[_deep-1].md5, sizeof(_path_info[_deep].md5)) &&
		!memcmp(_path_info[_deep].md5, _path_info[_deep-2].md5, sizeof(_path_info[_deep].md5)))
		return true;
	return false;
}

void Path::clearList(bool doDelete) {
	std::list<Dirent *> *_list = _path_info[_deep]._list;
	if (NULL == _list) return;
	std::list<Dirent *>::iterator it;
	for (it = _list->begin(); it != _list->end(); it++)
		delete *it;
	_list->clear();
	if (!doDelete) return;
	delete _path_info[_deep]._list;
	_path_info[_deep]._list = NULL;
}

void Path::createList() {
	_path_info[_deep]._list = new std::list<Dirent *>;
}

void Path::clearDirs(bool doDelete) {
	std::list<Dirent *> *_list = _path_info[_deep]._dirs;
	if (NULL == _list) return;
/*	std::list<Dirent *>::iterator it;
	for (it = _list->begin(); it != _list->end(); it++)
		delete *it;*/
	_list->clear();
	if (!doDelete) return;
	delete _path_info[_deep]._dirs;
	_path_info[_deep]._dirs = NULL;
}

void Path::createDirsList() {
	_path_info[_deep]._dirs = new std::list<Dirent *>;
}

void Path::createLists() {
	createList();
	createDirsList();
}

uint64_t Path::setList(std::vector<Dirent *> &list) {
	uint64_t total_size = 0;
	std::list<Dirent *> *_list = _path_info[_deep]._list;
	struct MD5Context context;
	std::vector<Dirent *>::iterator it;

	MD5Init(&context);
	for (it = list.begin(); it != list.end(); it++) {
		Dirent *item = *it;
		_list->push_back(item);
		MD5Update(&context, (const uint8_t *)item->name(), item->namelen());
		if (item->type() == TYPE_DIRECTORY)
			continue;
		uint64_t size = item->size();
		time_t mtime = item->mtime();
		MD5Update(&context, (const uint8_t *)&size, sizeof(size));
		MD5Update(&context, (const uint8_t *)&mtime, sizeof(mtime));
		total_size += size;
	}

	MD5Final(_path_info[_deep].md5, &context);
	return total_size;
}

std::list<Dirent *> *Path::getDirs() {
	return _path_info[_deep]._dirs;
}

std::list<Dirent *> *Path::getList() {
	return _path_info[_deep]._list;
}

bool Path::loadFromDB() {
	MYSQL_RES *res;

	if (_db.query("SELECT uid FROM paths WHERE share=%u AND p2f=%u", _share, p2f())
			&& (res = _db.results())) {
		MYSQL_ROW row = mysql_fetch_row(res);
		if (row)
			_path_info[_deep].uid = atoi(row[0]);
		mysql_free_result(res);
	}

	if (uid() < 1)
		return insertIntoDB();
	return true;
}

bool Path::insertIntoDB() {
	char *path = _db.escape(_path);
	_path_info[_deep].uid = _db.insert("INSERT INTO paths VALUES(NULL,'%s',%u,%u)", path, _share, p2f());
	delete [] path;
	return (_path_info[_deep].uid > 0);
}

void Path::drop(const char *name) {
	size_t length = strlen(name);
	size_t oldlen = _path_info[_deep].length;
	if (oldlen + length > MAX_PATH)
		return;

	if (length) {
		memcpy(_path + oldlen, name, length);
		size_t newlen = oldlen + length;
		_path[newlen++] = '/';
		_path[newlen] = '\0';
	}

	char *full_path = _db.escape(_path);
	_path[oldlen] = '\0';

	_db.query("DELETE paths2files FROM paths, paths2files WHERE share=%u AND name LIKE '%s%%' AND path=paths.uid", _share, full_path);
	_db.query("DELETE FROM paths WHERE share=%u AND name like '%s%%'", _share, full_path);
	delete [] full_path;
}

void Path::updateInDB() {
	if (!loadFromDB())
		return;

	MYSQL_RES *res;
	MYSQL_ROW row;

	bool row_changed = false;
	char *name;
	unsigned char type;
	unsigned int uid;

	bool delete_new_items = false;
	std::list<Dirent *> *new_items;
	std::list<Dirent *> *list = getList();
	std::list<Dirent *> *dirs = getDirs();
	std::list<unsigned int> free_ids;
	std::list<Dirent *>::iterator it = list->begin();

	if (!_db.query("SELECT p2f.uid,f.uid,UCASE(name),type,size,mtime,add_info from paths2files p2f,files f WHERE path=%u AND file=f.uid ORDER BY UCASE(name)", this->uid())
			|| !(res = _db.results())) {
		new_items = list;
		goto add_new_files;
	}

	if ((row = mysql_fetch_row(res)))
		row_changed = true;

	new_items = new std::list<Dirent *>;
	delete_new_items = true;


	while (row || it != list->end()) {
		bool do_delete = false;
		int cmp;
		if (row && row_changed) {
			uid = atoi(row[0]); name = row[2];
			type = atoi(row[3]); row_changed = false;
		}

		if (row && it != list->end()) cmp = strcmp((*it)->upname(), name);
		else if (row) cmp = 1;
		else cmp = -1;

		if (cmp == 0) { // File names equals
			Dirent *file = *it;
			if ((file->type() == TYPE_DIRECTORY && type == TYPE_DIRECTORY) ||
				(file->type() != TYPE_DIRECTORY && type != TYPE_DIRECTORY)) { // Name and type equals
				uint64_t size = atoll(row[4]);

				if (type == TYPE_DIRECTORY) {
					file->uid(uid); file->size(size);
					file->add_info(atoi(row[6]));

					dirs->push_back(file);
				} else {
					if (file->size() == size) {
						if (file->mtime() != atoi(row[5])) // DST change bug?
							file->updateInDB(uid);
					} else file->updateInDB(uid, time(NULL));
				}
			} else {
				do_delete = true;
				new_items->push_back(file); //insert new into db
			}

			it++;
			row_changed = true;
		} else if (cmp > 0) {
			do_delete = true;
			row_changed = true;
		} else { // cmp < 0
			new_items->push_back(*it); // insert new into db
			it++;
		}

		if (do_delete) {
			free_ids.push_back(uid);

			if (type == TYPE_DIRECTORY)
				drop(name);
		}

		if (row_changed)
			row = mysql_fetch_row(res);
	}
	mysql_free_result(res);

add_new_files:

	std::list<unsigned int>::iterator del = free_ids.begin();
	for (it = new_items->begin(); it != new_items->end(); it++) {
		Dirent *item = *it;

		unsigned int del_uid = 0;
		if (del != free_ids.end()) {
			del_uid = *del;
			del++;
		}

		item->path(this->uid());
		if (!item->insertIntoDB(del_uid))
			continue;

		if (item->type() == TYPE_DIRECTORY)
			dirs->push_back(item);
	}

	while (del != free_ids.end()) {
		_db.query("DELETE FROM paths2files WHERE uid=%u", *del);
		del++;
	}

	if (delete_new_items)
		delete new_items;
}
