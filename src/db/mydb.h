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
#ifndef MYSQLDB_H_
#define MYSQLDB_H_

#include <stdint.h>
#include <stdarg.h>
#include <mysql.h>

class MyDB {
private:
	MYSQL _mysql;
	void connect(void) throw (std::string);
	bool query(const char *formatstr, va_list &ap);

public:
	char *escape(const char *string);
	bool query(const char *formatstr, ...);
	uint64_t insert(const char *formatstr, ...);
	MYSQL_RES *results(void) { return mysql_store_result(&_mysql); }
	MyDB() { connect(); }
	virtual ~MyDB() { mysql_close(&_mysql); }
};

#endif /* MYSQLDB_H_ */
