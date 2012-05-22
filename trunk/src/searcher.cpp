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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <sys/time.h>

#include "libsearch/searcher.h"

int main(int argc, char **argv) {
	char type = '0';
	struct timeval tb, te;
	struct timezone tz;

	if (argc < 2) {
		printf("Usage: ./searcher <query> [type]\n");
		return 0;
	}
	if (argc > 2)
		type = argv[2][0];
	setlocale(LC_CTYPE, "ru_RU.KOI8-R");
	fflush(stdout);
	gettimeofday(&tb, &tz);
	CSearcher s("fls.txt");

	char *res = s.doQuery(type, argv[1]);
	if (res) {
		printf("%s\n", res);
		if (*res)
			delete [] res;
	} else
		printf("No results\n");
	gettimeofday(&te, &tz);

	printf("Search time: %f\n", te.tv_sec + te.tv_usec / 1000000.0 - tb.tv_sec - tb.tv_usec / 1000000.0);
}
