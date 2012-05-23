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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "libcgi.h"

CGI::CGI() {
	query_string = (char *) NULL;
	getQueryString();
}

CGI::~CGI() {
	if (query_string)
		delete query_string;
}

const char *CGI::getRequestMethod() {
	char *ret = getenv((char *) "REQUEST_METHOD");
	if (!ret)
		return "GET";
	return ret;
}

char *CGI::param(const char *name) {
	char *res, *rres;
	char *s, *x;
	char a, b;
	if (!query_string)
		return (char *) NULL;
	if (!name)
		return (char *) NULL;
	if ((s = strstr(query_string, name)) == NULL)
		return NULL;
	rres = res = (char *) malloc(strlen(query_string));
	if ((x = strchr(s, '&')) != NULL)
		x[0] = 0;
	s = s + strlen(name);
	while (*s != '\0') {
		if (*s == '%' && x > s + 2) {
			if ((a = parsesym(*(s + 1))) == 20) {
				s++;
				continue; // ignore invalid %
			}
			if ((b = parsesym(*(s + 2))) == 20) {
				s++;
				continue; // ignore invalid %
			}
			*res = a * 16 + b;
			s += 2;
		} else if (*s == '+')
			*res = ' ';
		else
			*res = *s;
		res++;
		s++;
	}
	if (x)
		x[0] = '&';
	*res = 0;
	rres = (char*) realloc(rres, strlen(rres) + 1);
	return rres;
}

int CGI::int_param(const char *name) {
	int ret = -1;
	char *tmp = param(name);
	if (tmp) {
		ret = atoi(tmp);
		if (!ret && strcmp(tmp, "0"))
			ret = -1;
		free(tmp);
	}
	return ret;
}

int64_t CGI::long_param(const char *name) {
	int64_t ret = -1;
	char *tmp = param(name);
	if (tmp) {
		ret = atol(tmp);
		if (!ret && strcmp(tmp, "0"))
			ret = -1;
		free(tmp);
	}
	return ret;
}

char *CGI::getQueryString(void) {
	if (query_string)
		return query_string;
	if (!strcmp(getRequestMethod(), "POST"))
		getPOSTString();
	else
		getGETString();
	return query_string;
}

void CGI::getGETString(void) {
	char *tmp = getenv("QUERY_STRING");
	if (!tmp)
		tmp = (char *) "";
	query_string = new char[strlen(tmp) + 1];
	strcpy(query_string, tmp);
}

void CGI::getPOSTString(void) {
	char *tmp = getenv("CONTENT_LENGTH");
	int len = 0;
	if (tmp)
		len = atoi(tmp);
	query_string = new char[len + 1];
	int readed = fread(query_string, 1, len, stdin);
	query_string[readed] = '\0';
}

/* parse hex symbol to value */
char CGI::parsesym(char s) {
	s = (char) toupper(s);
	if (s >= '0' && s <= '9')
		return s - '0';
	if (s >= 'A' && s <= 'F')
		return s - 'A' + 10;
	return 20;
}

