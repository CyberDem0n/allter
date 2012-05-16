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

#include "indexer.h"
#include "substrings.h"
#include "searcher.h"
#include "merge.h"

CIndexer::CIndexer(int size) {
	dwords_size = size;
	CSubstrings::init();
	init_dwords();
}

CIndexer::~CIndexer() {
	close(f);
}

void CIndexer::init_dwords(void) {
	dwords = new unsigned long long[dwords_size];
	if (!dwords) {
		printf("`new' error!\n");
		init = 0;
	} else
		init = 1;

	f = open("tmp.bin", O_RDWR);
	if (f == -1) {
		dwc = 0;
		f = creat("tmp.bin", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	} else {
		dwc = (int) (lseek(f, 0, SEEK_END) / sizeof(unsigned long long));
		int num_blocks = (int) ((dwc - 1) / dwords_size);
		unsigned int block_size = dwc - dwords_size * num_blocks;
		if (block_size < dwords_size) {
			lseek(f, num_blocks * dwords_size * sizeof(unsigned long long), SEEK_SET);
			if (read(f, dwords, sizeof(unsigned long long) * block_size) == (ssize_t) sizeof(unsigned long long) * block_size);
			lseek(f, num_blocks * dwords_size * sizeof(unsigned long long), SEEK_SET);
		} else {
			dwc = 0;
			lseek(f, 0, SEEK_END);
		}
	}
}

void CIndexer::addString(unsigned char *str, unsigned int offset, unsigned int length) {
	int len;
	CSubstrings s(str, length);
	for (int i = 0; i < s.wc; i++) {
		len = strlen((const char *) s[i]) - 3;
		for (int j = 0; j <= len; j++) {
			dwords[dwc % dwords_size] = (((unsigned long long) s[i][j]) << 56) | (((unsigned long long) s[i][j + 1]) << 48)	| (((unsigned long long) s[i][j + 2]) << 40) | ((unsigned long long) offset);
			if (j < len)
				dwords[dwc % dwords_size] |= (((unsigned long long) s[i][j + 3]) << 32);
			dwc++;
			if (dwc % dwords_size == 0) {
				quicksort(0, dwords_size - 1);
				if (write(f, dwords, sizeof(unsigned long long) * dwords_size) == (ssize_t) sizeof(unsigned long long) * dwords_size);
			}
		}
	}
}

void CIndexer::sort(void) {
	ssize_t size = dwc % dwords_size;
	quicksort(0, size - 1);
	size *= sizeof(unsigned long long);
	if (write(f, dwords, size) == size);
	delete[] dwords;
	close(f);
	f = open("tmp.bin", O_RDONLY);
}

inline int CIndexer::partition(int p, int r) {
	unsigned long long x, i, j, tmp;

	x = dwords[(int) ((p + r) / 2)];
	i = p - 1;
	j = r + 1;

	while (1) {
		do {
			--j;
		} while (dwords[j] > x);

		do {
			++i;
		} while (dwords[i] < x);

		if (i < j) {
			tmp = dwords[i];
			dwords[i] = dwords[j];
			dwords[j] = tmp;
		} else
			return j;
	}
}

void CIndexer::quicksort(int p, int r) {
	int q;

	if (p < r) {
		q = partition(p, r);
		quicksort(p, q);
		quicksort(q + 1, r);
	}
}

void CIndexer::iwrite(char *fname_base) {
	FILE *id2, *id4, *idx;
	unsigned int temp, prev1, prev4, prev4_offset, prev_offset, idx_count, id4_count;
	size_t buf_size = 32768;
	unsigned long long dwrds, prev = 0;
	int len = strlen(fname_base);
	char *id2_name = new char[len + 6];
	char *id4_name = new char[len + 6];
	char *idx_name = new char[len + 6];

	struct index4 *buffer4 = new struct index4[buf_size];
	unsigned int *bufferx = new unsigned int[buf_size];

	strcpy(id2_name, fname_base);
	strcat(id2_name, ".id2t");
	strcpy(id4_name, fname_base);
	strcat(id4_name, ".id4t");
	strcpy(idx_name, fname_base);
	strcat(idx_name, ".idxt");

	id2 = fopen(id2_name, "wb");
	id4 = fopen(id4_name, "wb");
	idx = fopen(idx_name, "wb");

	delete[] id2_name;
	delete[] id4_name;
	delete[] idx_name;

	prev1 = prev4 = prev4_offset = prev_offset = idx_count = id4_count = 0;

	CMerge *c = new CMerge(f, dwords_size, dwords_size*sizeof(unsigned long long));

	for (unsigned int i = 0; i < dwc; i++) {
		dwrds = c->get_element();
		if (prev != dwrds) {
			prev = dwrds;
			bufferx[idx_count % buf_size] = (unsigned int) prev;
			temp = (unsigned int) (prev >> 32);
			if (prev4 != temp) {
				if (prev4 != 0) {
					buffer4[(id4_count - 1) % buf_size].size = idx_count - prev_offset;
					if (id4_count % buf_size == 0)
						if (fwrite(buffer4, sizeof(struct index4), buf_size, id4) == buf_size);
				}
				prev4 = temp;
				buffer4[id4_count % buf_size].chr = (unsigned short) prev4;
				buffer4[id4_count % buf_size].offset = idx_count << 2;
				prev_offset = idx_count;

				temp = (unsigned int) (prev >> 48);
				if (prev1 != temp) {
					if (prev1 != 0) {
						prev4_offset = id4_count - prev4_offset;
						if (fwrite(&prev4_offset, sizeof(unsigned int), 1, id2)	== 1);
					}
					prev1 = temp;
					prev4_offset = id4_count * sizeof(struct index4);
					fseek(id2, temp * 2 * sizeof(unsigned int), SEEK_SET);
					if (fwrite(&prev4_offset, sizeof(unsigned int), 1, id2)	== 1);
					prev4_offset = id4_count;
				}
				id4_count++;
			}
			idx_count++;
			if (idx_count % buf_size == 0)
				if (fwrite(bufferx, sizeof(unsigned int), buf_size, idx)== buf_size);
		}
	}

	if (fwrite(bufferx, sizeof(unsigned int), idx_count % buf_size, idx) == idx_count % buf_size);

	buffer4[(id4_count - 1) % buf_size].size = idx_count - prev_offset;
	if (fwrite(buffer4, sizeof(struct index4), id4_count % buf_size, id4) == id4_count % buf_size);

	prev4_offset = id4_count - prev4_offset;
	if (fwrite(&prev4_offset, sizeof(unsigned int), 1, id2) == 1);

	delete c;

	delete[] buffer4;
	delete[] bufferx;

	fclose(idx);
	fclose(id4);
	fclose(id2);
}

