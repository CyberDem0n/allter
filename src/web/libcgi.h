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
#ifndef __LIBCGI_H__
#define __LIBCGI_H__

class CGI {
private:
	char *query_string;
	char parsesym(char);
	void getPOSTString(void);
	void getGETString(void);
public:
	CGI();
	~CGI();
	const char *getRequestMethod(void);
	char *getQueryString(void);
	char *param(const char *);
	int int_param(const char *);
	long long long_param(const char *);
};

#endif // __LIBCGI_H__
