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
#ifndef PATH_H_
#define PATH_H_

#include <list>
#include <vector>

#include "dirent.h"
#include "../db/mydb.h"

#define MAX_PATH 1024
#define MAX_DEEP 32

class Path {
private:
	MyDB &_db;
	unsigned int _share;
	int _deep;
	char _path[MAX_PATH];
	size_t _pathlen;
	struct path_info {
		unsigned int uid, p2f;
		size_t length;
		uint8_t md5[16];
		std::list<Dirent *> *_list;
		std::list<Dirent *> *_dirs;
	} _path_info[MAX_DEEP];

	bool loadFromDB();
	bool insertIntoDB();
	void clearList(bool doDelete = true);
	void createList();
	void clearDirs(bool doDelete = true);
	void createDirsList();
	void createLists();

public:
	Path(MyDB &db, unsigned int share);

	virtual ~Path();

	void drop(const char *name);
	unsigned int uid() { return _path_info[_deep].uid; }
	unsigned int p2f() { return _path_info[_deep].p2f; }
	void updateInDB();
	bool goDown(unsigned int p2f, const char *name);
	bool goUp();
	bool detectSelfLooping();
	const char *path() { return _path; }
	uint64_t setList(std::vector<Dirent *> &list);
	std::list<Dirent *> *getDirs();
	std::list<Dirent *> *getList();
};

#endif /* PATH_H_ */
