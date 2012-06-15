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
#ifndef __SEARCHER_H__
#define __SEARCHER_H__

#include "substrings.h"

struct index2 {
	unsigned int id4_offset;
	unsigned int id4_size;
}__attribute__((packed));

struct index4 {
	unsigned short word;
	unsigned int offset;
	unsigned int size;
}__attribute__((packed));

class CSearcher {
private:
	struct index2 *_id2;
	unsigned int _id2_size;
	struct index4 *_id4;
	unsigned int _id4_size;

	const char *base_name;
	char *id2_name, *id4_name, *idx_name;

	const int max_results; // Maximum number of returned id's

	void *mapFile(const char *, off_t, size_t &, size_t &);
	bool mapIndexes(void);
	void unmapIndexes(void);
	bool remapIndexes(void);

	int check_query(const char *);

	bool mstrstr(const char *, CSubstrings &);
	bool wstrstr(const char *, CSubstrings &);
	bool bstrstr(const char *, CSubstrings &);

	int binarySearch(unsigned int, unsigned int, unsigned short, bool);
	bool find_chain4(const unsigned char *, unsigned int, unsigned int, unsigned int &, unsigned int &);
	bool find_chain3(const unsigned char *, unsigned int, unsigned int, unsigned int &, unsigned int &);
public:
	CSearcher(const char *);
	~CSearcher();
	char *doQuery(const char, const char *);
};

#endif // __SEARCHER_H__
