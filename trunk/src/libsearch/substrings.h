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
	static char good_chars[256];
	static int good_chars_inited;

	struct substrings {
		const char *string;
		size_t length;
	} *_substrings;
	char *_substrings_container;
	void getSubstrings(void);
	int _max_len_id;
	int _size;
	size_t _max_len;
public:
	const size_t length;
	const char *string;
	CSubstrings(const char *);
	CSubstrings(const char *, size_t);
	~CSubstrings();
	static void init(void);
	const char *stringAt(int);
	const size_t strlenAt(int);
	const int &size;
	const size_t &max_len;
};

#endif // __SUBSTRINGS_H__
