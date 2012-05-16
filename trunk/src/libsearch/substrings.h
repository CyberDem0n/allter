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
#ifndef __SUBSTRINGS_H__
#define __SUBSTRINGS_H__
class CSubstrings {
private:
	unsigned char *s;
	unsigned char **substrings;
	unsigned int len;
	void getSubstrings(void);
public:
	CSubstrings(unsigned char *);
	CSubstrings(unsigned char *, unsigned int);
	~CSubstrings();
	static void init(void);
	unsigned char * operator [](int);
	int wc;
	int max_len;
	int max_len_id;
};

#endif // __SUBSTRINGS_H__
