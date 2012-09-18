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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <limits.h>

#include "merge.h"

CMerge::CMerge(int fd, int blk_size, int mem_size) : _fd(fd), elemen_count(0), block_size(blk_size) {
	num_elements = (int) (lseek(_fd, 0, SEEK_END) / sizeof(uint64_t));
	num_blocks = (int) ((num_elements - 1) / block_size);
	blocks = new struct block[num_blocks + 1];
	buffer_size = (int) (mem_size / sizeof(uint64_t) / (num_blocks + 1)); // Buffer size (in elements)
	if (buffer_size == 0)
		buffer_size = 1;
	for (int i = 0; i < num_blocks; i++)
		init_block(i, block_size);
	init_block(num_blocks, num_elements - block_size*num_blocks);
	num_blocks++;

	printf("num_blocks=%d, buffer_size=%lu\n", num_blocks, buffer_size);
}

CMerge::~CMerge() {
	for (int i = 0; i < num_blocks; i++)
		if (blocks[i].mmaped_buf != NULL)
			munmap(blocks[i].mmaped_buf, blocks[i].mmaped_len);
	delete[] blocks;
}

void CMerge::init_block(int block_num, int blk_size) {
	struct block *block = blocks + block_num;
	block->mmaped_buf = NULL;
	block->block_offset = block_num * block_size;
	block->buffer_offset = 0;
	block->block_size = blk_size;
	block->is_over = false;
	read_block(block_num);
}

void CMerge::read_block(int block_num) {
	struct block *block = blocks + block_num;
//	printf("%d %d %d %d\n", block_num,block_size ,blocks[block_num].buffer_count,buffer_size);
	if (block->mmaped_buf != NULL)
		munmap(block->mmaped_buf, block->mmaped_len);

	int offset = (block->block_offset + block->buffer_offset) * sizeof(uint64_t);
	int map_align = offset % getpagesize();

	if (block->block_size - block->buffer_offset < buffer_size) {
		block->buffer_size = block->block_size - block->buffer_offset;
	} else block->buffer_size = buffer_size;

	block->mmaped_len = sizeof(uint64_t) * block->buffer_size + map_align;

	block->mmaped_buf = mmap(NULL, block->mmaped_len, PROT_READ, MAP_SHARED, _fd, offset - map_align);
	block->buffer = (uint64_t *) ((char *)block->mmaped_buf + map_align);
	block->buffer_offset += buffer_size;
	block->buffer_cursor = 0;
}

uint64_t CMerge::get_element(void) {
	struct block *block;
	uint64_t min_element = ULONG_LONG_MAX;
	int block_num = -1;

	if (++elemen_count > num_elements)
		return min_element;

	for (int i = 0; i < num_blocks; i++) {
		block = blocks + i;
		if (block->is_over || block->buffer[block->buffer_cursor] >= min_element)
			continue;
		min_element = block->buffer[block->buffer_cursor];
		block_num = i;
	}

	if (block_num < 0) return min_element;

	block = blocks + block_num;

	if (++block->buffer_cursor >= (off_t)block->buffer_size) {
		if (block->buffer_offset < (off_t)block->block_size)
			read_block(block_num);
		else block->is_over = true;
	}

	return min_element;
}
