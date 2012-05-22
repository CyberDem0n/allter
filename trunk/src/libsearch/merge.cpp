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

#include "merge.h"

CMerge::CMerge(int fil, int blk_size, int mem_size) {
	fd = fil;
	block_size = blk_size;
	num_elements = (int) (lseek(fd, 0, SEEK_END) / sizeof(unsigned long long));
	elemen_count = 0;
	num_blocks = (int) ((num_elements - 1) / block_size) + 1;
	blocks = new struct block[num_blocks];
	buffer_size = (int) (mem_size / sizeof(unsigned long long) / num_blocks); // Buffer size (in elements)
	if (buffer_size == 0)
		buffer_size = 1;
	for (int i = 0; i < num_blocks - 1; i++)
		init_block(i, block_size);
	init_block(num_blocks - 1, num_elements - block_size * (num_blocks - 1));

	printf("num_blocks=%d, buffer_size=%d\n", num_blocks, buffer_size);
}

CMerge::~CMerge() {
	for (int i = 0; i < num_blocks; i++)
		if (blocks[i].inited)
			munmap(blocks[i].tmp_buf, blocks[i].mmaped_len);
	delete[] blocks;
}

void CMerge::init_block(int block_num, int block_size) {
	struct block *block = blocks + block_num;
	block->tmp_buf = NULL;
	block->inited = 0;
	block->block_size = block_size;
	block->buffer_size = buffer_size;
	block->buffer_count = 0;
	block->is_over = 0;
	read_block(block_num);
}

void CMerge::read_block(int block_num) {
	struct block *block = blocks + block_num;
//	printf("%d %d %d %d\n", block_num,block_size ,blocks[block_num].buffer_count,buffer_size);
	if (block->inited)
		munmap(block->tmp_buf, block->mmaped_len);

	int offset = (block_num * block_size + block->buffer_count * buffer_size) * sizeof(unsigned long long);
	int map_align = offset % getpagesize();

	if (block->block_size - block->buffer_count * buffer_size < buffer_size) {
		block->buffer_size = block->block_size - block->buffer_count * buffer_size;
		block->is_over = 1;
	}

	block->mmaped_len = sizeof(unsigned long long) * block->buffer_size + map_align;

	block->tmp_buf = mmap(NULL, block->mmaped_len, PROT_READ, MAP_SHARED, fd, offset - map_align);
	block->buffer = (unsigned long long *) ((char *)block->tmp_buf + map_align);
	block->inited = 1;
	block->buffer_count++;
	block->buffer_cursor = 0;
}

unsigned long long CMerge::get_element(int block_num) {
	struct block *block = blocks + block_num;
	if (block->is_over && block->buffer_cursor >= block->buffer_size)
		return 0xFFFFFFFFFFFFFFFFLL;
	return block->buffer[blocks->buffer_cursor];
}

unsigned long long CMerge::pop_element(int block_num) {
	struct block *block = blocks + block_num;
	unsigned long long ret = block->buffer[block->buffer_cursor];
	if (++block->buffer_cursor >= block->buffer_size && block->is_over != 1)
		read_block(block_num);
	return ret;
}

unsigned long long CMerge::get_element(void) {
	if (++elemen_count > num_elements)
		return 0xFFFFFFFFFFFFFFFFLL;
	unsigned long long element, min_element = 0xFFFFFFFFFFFFFFFFLL;
	int element_id = -1;
	for (int i = 0; i < num_blocks; i++)
		if ((element = get_element(i)) < min_element) {
			min_element = element;
			element_id = i;
		}
//	printf("%d %d\n",element_id, (int)min_element);
	return pop_element(element_id);
}
/*
 int main()
 {
 FILE *f = fopen("test.bin","wb");
 unsigned long long *tmp = new unsigned long long[10];
 for (int j=0; j<3; j++) {
 for (int i=0; i<10; i++)
 tmp[i] = i;
 fwrite(tmp, sizeof(unsigned long long), 10, f);
 }
 for (int i=0; i<7; i++)
 tmp[i] = i;
 fwrite(tmp, sizeof(unsigned long long), 7, f);

 f = freopen("test.bin", "rb", f);
 CMerge c(f, 10, 129);
 for (int i=0; i<45; i++)
 printf("%d\n",(int)c.get_element());
 fclose(f);
 }*/
