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
	size_t block_size;
	off_t block_offset;
	void *mmaped_buf;
	uint64_t *buffer;
	off_t buffer_offset;
	size_t buffer_size;
	off_t buffer_cursor;
	int mmaped_len;
	bool is_over;
};

class CMerge {
private:
	int _fd;
	unsigned int num_elements;
	unsigned int elemen_count;
	int num_blocks;
	size_t block_size;
	size_t buffer_size;
	struct block *blocks;
	void init_block(int, int);
	void read_block(int);
public:
	CMerge(int, int, int);
	~CMerge();
	uint64_t get_element(void);
};

#endif // __MERGE_H__
