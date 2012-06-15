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
#ifndef __INDEXER_H__
#define __INDEXER_H__

#include <stdint.h>

class CIndexer {
private:
	const char *base_name;
	int f;
	unsigned int dwc;
	unsigned int dwords_size;
	uint64_t *dwords;
	int base_name_len;
	char *name_tmp;
	void initBlock(void);
	void saveBlock(ssize_t size);
public:
	CIndexer(const char *, int);
	~CIndexer();
	void addString(const char *, unsigned int, size_t);
	void iwrite();
};

#endif // __INDEXER_H__
