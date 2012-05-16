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
class CIndexer {
private:
	int f;
	int init;
	unsigned int dwc;
	unsigned int dwords_size;
	unsigned long long *dwords;
	inline int partition(int, int);
	void quicksort(int, int);
	void init_dwords(void);
public:
	CIndexer(int);
	~CIndexer();
	void addString(unsigned char *, unsigned int, unsigned int);
	void sort(void);
	void iwrite(char *);
};

#endif // __INDEXER_H__
