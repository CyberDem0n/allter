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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <byteswap.h>
#include <assert.h>

#include "indexer.h"
#include "substrings.h"
#include "searcher.h"
#include "merge.h"

CIndexer::CIndexer(const char *name, int size) : base_name(name), dwords_size(size) {
	CSubstrings::init();
	base_name_len = strlen(base_name);
	name_tmp = new char [base_name_len + 6];
	strcpy(name_tmp, base_name);
	strcpy(name_tmp + base_name_len, ".bin");
	initBlock();
}

CIndexer::~CIndexer() {
	delete [] name_tmp;
	close(f);
}

void CIndexer::initBlock(void) {
	dwords = new uint64_t[dwords_size];
	if (!dwords) {
		perror("Unable allocate memory...");
		exit(1);
	}

	f = open(name_tmp, O_RDWR);
	if (f == -1) {
		dwc = 0;
		f = creat(name_tmp, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	} else {
		dwc = (int) (lseek(f, 0, SEEK_END) / sizeof(uint64_t));
		int num_blocks = (int) ((dwc - 1) / dwords_size);
		unsigned int block_size = dwc - dwords_size * num_blocks;
		if (block_size < dwords_size) {
			lseek(f, num_blocks * dwords_size * sizeof(uint64_t), SEEK_SET);
			if (read(f, dwords, sizeof(uint64_t) * block_size) != (ssize_t) sizeof(uint64_t) * block_size) {
				perror("Unable init last block...");
				exit(1);
			}
			lseek(f, num_blocks * dwords_size * sizeof(uint64_t), SEEK_SET);
		} else {
			dwc = 0;
			lseek(f, 0, SEEK_END);
		}
	}
}

void CIndexer::addString(const char *string, unsigned int offset, size_t length) {
	CSubstrings sub(string, length);

	for (size_t i = 0; i < sub.size; i++) {
		const unsigned char *str = sub.indexAt(i);
		int len = sub.indexlenAt(i) - 3;

		for (int j = 0; j <= len; j++) {
			dwords[dwc % dwords_size] = ((uint64_t)bswap_32(*((unsigned int *)(str+j))) << 32) | ((uint64_t) offset);

			if ((++dwc) % dwords_size == 0)
				saveBlock(dwords_size);
		}
	}
}

static int compare(const void *elem1, const void *elem2) {
	uint64_t *p1 = (uint64_t *)elem1;
	uint64_t *p2 = (uint64_t *)elem2;
	if (*p1 == *p2) return 0;
	if (*p1 < *p2) return -1;
	return 1;
}

void CIndexer::saveBlock(ssize_t size) {
	qsort(dwords, size, sizeof(uint64_t), compare);

	size *= sizeof(uint64_t);

	if (write(f, dwords, size) != size) {
		perror("Unable write temporary data...");
		exit(1);
	}
}

void CIndexer::iwrite() {
	FILE *id2, *id4, *idx;
	unsigned int temp, prev1, prev4, prev4_offset, prev_offset, idx_count, id4_count;
	size_t buf_size = 32768;
	uint64_t dwrds, prev = 0;

	struct index4 *buffer4 = new struct index4[buf_size];
	unsigned int *bufferx = new unsigned int[buf_size];

	saveBlock(dwc % dwords_size);

	delete[] dwords;
	close(f);
	f = open(name_tmp, O_RDONLY);

	strcpy(name_tmp + base_name_len, ".id2t"); id2 = fopen(name_tmp, "wb");
	strcpy(name_tmp + base_name_len, ".id4t"); id4 = fopen(name_tmp, "wb");
	strcpy(name_tmp + base_name_len, ".idxt"); idx = fopen(name_tmp, "wb");

	CMerge *c = new CMerge(f, dwords_size, dwords_size*sizeof(uint64_t));

	prev1 = prev4 = prev4_offset = prev_offset = idx_count = id4_count = 0;

	for (unsigned int i = 0; i < dwc; i++) {
		dwrds = c->get_element();
		if (prev == dwrds)
			continue;

		prev = dwrds;
		bufferx[idx_count % buf_size] = (unsigned int) prev;
		temp = (unsigned int) (prev >> 32);
		if (prev4 != temp) {
			if (prev4 != 0) {
				buffer4[(id4_count - 1) % buf_size].size = idx_count - prev_offset;
				if (id4_count % buf_size == 0)
					assert(fwrite(buffer4, sizeof(struct index4), buf_size, id4) == buf_size);
			}
			prev4 = temp;
			buffer4[id4_count % buf_size].word = (unsigned short) prev4;
			buffer4[id4_count % buf_size].offset = idx_count << 2;
			prev_offset = idx_count;

			temp = (unsigned int) (prev >> 48);
			if (prev1 != temp) {
				if (prev1 != 0) {
					prev4_offset = id4_count - prev4_offset;
					assert(fwrite(&prev4_offset, sizeof(unsigned int), 1, id2) == 1);
				}
				prev1 = temp;
				prev4_offset = id4_count * sizeof(struct index4);
				fseek(id2, temp * 2 * sizeof(unsigned int), SEEK_SET);
				assert(fwrite(&prev4_offset, sizeof(unsigned int), 1, id2) == 1);
				prev4_offset = id4_count;
			}
			id4_count++;
		}

		if ((++idx_count) % buf_size == 0)
			assert(fwrite(bufferx, sizeof(unsigned int), buf_size, idx) == buf_size);
	}

	assert(fwrite(bufferx, sizeof(unsigned int), idx_count % buf_size, idx) == idx_count % buf_size);

	buffer4[(id4_count - 1) % buf_size].size = idx_count - prev_offset;
	assert(fwrite(buffer4, sizeof(struct index4), id4_count % buf_size, id4) == id4_count % buf_size);

	prev4_offset = id4_count - prev4_offset;
	assert(fwrite(&prev4_offset, sizeof(unsigned int), 1, id2) == 1);

	delete c;

	delete[] buffer4;
	delete[] bufferx;

	fclose(idx);
	fclose(id4);
	fclose(id2);
}

