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

struct index4 {
	unsigned short chr;
	unsigned int offset;
	unsigned int size;
}__attribute__((packed));

class CSearcher {
private:
	struct index4 *id4;
	unsigned int id4_len;
	const char *base_name;
	char *id2_name, *id4_name, *idx_name;
	unsigned char *query;
	int query_type, query_len;
	int done, max_results; // Maximum number of returned id's
	char ret[10000];
	unsigned char type;

	inline int mstrstr(unsigned char *);
	inline int wstrstr(unsigned char *);
	inline int bstrstr(unsigned char *);
	int check_query(unsigned char *);
	int find_chain4(unsigned char *, unsigned int *, unsigned int *);
	int find_chain3(unsigned char *, unsigned int *, unsigned int *);
	CSubstrings *s;
public:
	CSearcher(const char *, char *, char);
	~CSearcher();
	char *results(void);
};

#endif // __SEARCHER_H__
