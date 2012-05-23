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
#include <stdio.h>
#include <string.h>
#include <string>

#include "mydb.h"

#define SAFE_FREE(x) do { if ((x) != NULL) {free(x); x=NULL;} } while (0)

/*#define DB_HOST		"localhost"
#define DB_USER		"root"
#define DB_PASSWD	""
#define DB_BASE		"allter"*/

void MyDB::connect(void) throw (std::string) {
	mysql_init(&_mysql);
	if (!mysql_real_connect(&_mysql, DB_HOST, DB_USER, DB_PASSWD, DB_BASE, 0, NULL, 0))
		throw std::string(mysql_error(&_mysql));
	mysql_query(&_mysql, "SET NAMES UTF8");
}

char *MyDB::escape(const char *string) {
	int size = strlen(string);
	char *ret = new char[2*size+2];
	mysql_real_escape_string(&_mysql, ret, string, size);
	return ret;
}

bool MyDB::query(const char *formatstr, va_list &ap) {
	bool ret = true;
	char *statement;
	vasprintf(&statement, formatstr, ap);
	va_end(ap);
//	fprintf(stderr, "mysql_query:%s\n", statement);
	if (mysql_real_query(&_mysql, statement, strlen(statement))) {
		fprintf(stderr, "MYSQL_ERROR: %s\n", mysql_error(&_mysql));
		fprintf(stderr, "MYSQL_ERROR: %s\n", statement);
		ret = false;
	}
	SAFE_FREE(statement);
	return ret;
}

bool MyDB::query(const char *formatstr, ...) {
	va_list ap;
	va_start(ap, formatstr);
	return query(formatstr, ap);
}

uint64_t MyDB::insert(const char *formatstr, ...) {
	va_list ap;
	va_start(ap, formatstr);
	if (query(formatstr, ap))
		return mysql_insert_id(&_mysql);
	return 0;
}
