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
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <string.h>

#include "db/mydb.h"
#include "client/smbclient.h"
#include "structs/host.h"
#include "structs/share.h"
#include "structs/path.h"
#include "structs/dirent.h"

class Crawler {
private:
	IClient &_client;
	MyDB &_db;

	class cmp {
	public:
		bool operator()(const Dirent *a, const Dirent *b) {
			return (strcmp(a->upname(), b->upname()) < 0);
		}
	} compare;

	std::vector<Dirent *> getSortedList(const char *path) {
		std::vector<Dirent *> ret;
		std::vector<struct my_dirent> list = _client.getDirList(path);
		std::vector<struct my_dirent>::iterator it;

		for (it = list.begin(); it != list.end(); it++) {
			Dirent *item = new Dirent(_db, *it);
			if (item->isValid())
				ret.push_back(item);
			else delete item;
		}

		std::sort(ret.begin(), ret.end(), compare);
		return ret;
	}

	int scanRecurse(Path &path, uint64_t &total_size) {
		try {
			std::vector<Dirent *> list = getSortedList(path.path());

			total_size += path.setList(list);

			if (path.detectSelfLooping())
				return 2;

			path.updateInDB();

			std::list<Dirent *> *dirs = path.getDirs();
			std::list<Dirent *>::iterator it;
			for (it = dirs->begin(); it != dirs->end(); it++) {
				Dirent *item = *it;
				if (!item->scanAllowed() || !path.goDown(item->uid(), item->name()))
					continue;

				uint64_t dir_size = 0;
				int ret = scanRecurse(path, dir_size);
				if (ret > 1) {
					path.goUp();
					return ret - 1;
				} else if (ret == 1) {
					printf("SELF LOOPING %u:%s:%s\n", item->uid(), path.path(), item->name());
					item->forbidScan();
					path.drop("");
				}

				item->updateSizeInDB(dir_size);
				total_size += dir_size;

				if (!path.goUp())
					break;
			}
		} catch (std::string &e) {
			std::cerr << "Unable get list of files in: " << path.path() << ": " << e << std::endl;
		}
		return 0;
	}

	void scanShare(Share &share) {
		Path path(_db, share.uid());
		uint64_t total_size = 0;

		_client.setShare(share.name());

		scanRecurse(path, total_size);

		time_t t = time(NULL);
		share.updateInDB(t, t, total_size);
	}

	void scanHost(Host &host) throw (std::string) {
		_client.setUser(host.login());
		_client.setPassword(host.passwd());
		_client.setHost(host.ip());

		std::vector<std::string> list = _client.getShares();
		std::vector<std::string>::iterator it;
		for (it = list.begin(); it != list.end(); it++) {
			const char *name = it->c_str();
			if (0 == strcasecmp(name, "print$")) continue;

			std::cout << "SHARE:" << (*it) << std::endl;

			Share share(_db, host.uid(), *it);

			if (!share.loadFromDB())
				continue;

			if (!share.scanAllowed())
				continue;

			if (share.isOutdated()) {
				try {
					scanShare(share);
				} catch (std::string &e) {
					std::cerr << "Unable to scan share '" << name << "': " << e << std::endl;
				}
			} else share.updateInDB(time(NULL));
		}
	}

public:
	void scan(unsigned int uid, const char *ip) {
		Host host(_db, uid, ip);

		if (!host.loadFromDB())
			throw std::string("Host not found in database");

		if (!host.scanAllowed())
			throw std::string("Scan forbidden for host");

		try {
			scanHost(host);
		} catch (std::string &e) {
			std::cerr << "Unable to scan host " << uid << ": " << e << std::endl;
		}
	}

	Crawler(IClient &client, MyDB &db) : _client(client), _db(db) {}

	virtual ~Crawler() {}
};

int main(int argc, char **argv) {
	if (argc < 3) {
		std::cerr << "Usage: crawler <host id> <host ip>" << std::endl;
		return 1;
	}
	unsigned int uid = atoi(argv[1]);
	const char *ip = argv[2];

	setlocale(LC_ALL,"");

	MyDB *db;
	try {
		db = new MyDB();
	} catch (std::string &e) {
		std::cerr << "Could not connect to database: " << e << std::endl;
		return 1;
	}

	IClient *client;
	try {
		client = new SmbClient();
		client->setTimeout(1000); // 1 Sec
	} catch (std::string &e) {
		std::cerr << "Failed to initialize SmbClient: " << e << std::endl;
		delete db;
		return 2;
	}

	Crawler scan(*client, *db);
	try {
		scan.scan(uid, ip);
	} catch (std::string &e) {
		std::cerr << "Unable scan host: " << e << std::endl;
	}

	delete client;
	delete db;
	return 0;
}
