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
#ifndef __MERGE_H__
#define __MERGE_H__

#include <stdint.h>

struct block {
	void *tmp_buf;
	uint64_t *buffer;
	int block_size; //Size of sorted data block. For last block it can be less then for previous blocks
	int buffer_count; //Position it data block
	int buffer_cursor;
	int buffer_size;
	int is_over;
	int inited;
	int mmaped_len;
};

class CMerge {
private:
	int fd;
	unsigned int num_elements;
	unsigned int elemen_count;
	int num_blocks;
	int block_size;
	int buffer_size;
	struct block *blocks;
	void init_block(int, int);
	void read_block(int);
	uint64_t get_element(int);
	uint64_t pop_element(int);
public:
	CMerge(int, int, int);
	~CMerge();
	uint64_t get_element(void);
};

#endif // __MERGE_H__