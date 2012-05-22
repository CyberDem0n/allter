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
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

#include "libsearch/indexer.h"

#define MAX_STR_LEN 4096

void usage(char *prname) {
	printf("%s <file.txt> <last offset>\n", prname);
}

int main(int argc, char **argv) {
	FILE *f;
	char *tmp;
	char buf[MAX_STR_LEN];
	unsigned int offset = 0;
	CIndexer *idx;
	struct timeval tb, te;
	struct timezone tz;

	if (argc < 2) {
		usage(argv[0]);
		return 0;
	}
	gettimeofday(&tb, &tz);
	f = fopen(argv[1], "r");
	if (!f) {
		printf("Can't open file: %s\n", argv[1]);
		return 0;
	}

	offset = atol(argv[2]);
	fseek(f, offset, SEEK_SET);

	idx = new CIndexer(argv[1], 36000000); /* was 12000000 A5B !!! */

	while (fgets(buf, MAX_STR_LEN, f)) {
//		printf("%s",buf);
		tmp = strchr(buf, ',') + 3;
		idx->addString(tmp, offset, strlen(tmp));
		offset = ftell(f);
	}
	fclose(f);

	gettimeofday(&te, &tz);

	printf("Read time: %f\n", te.tv_sec + te.tv_usec / 1000000.0 - tb.tv_sec - tb.tv_usec / 1000000.0);

	gettimeofday(&tb, &tz);
	idx->iwrite();
	gettimeofday(&te, &tz);

	printf("Write time: %f\n", te.tv_sec + te.tv_usec / 1000000.0 - tb.tv_sec - tb.tv_usec / 1000000.0);

	delete idx;
}
