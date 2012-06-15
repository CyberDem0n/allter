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
	static unsigned char good_chars[65536];
	static bool good_chars_inited;

	bool _only_index;
	struct substrings {
		const unsigned char *index;
		const char *string;
		size_t length;
		size_t index_length;
	} *_substrings;
	unsigned char *_indexes_container;
	char * _substrings_container;
	void getSubstrings();
	int _max_len_id;
	size_t _size;
	size_t _max_len;
public:
	const size_t length;
	const char *string;
	CSubstrings(const char *);
	CSubstrings(const char *, size_t);
	~CSubstrings();
	static bool initCharsetTable(const char *charset_table);
	static void init(void);
	const char *stringAt(unsigned int);
	const unsigned char *indexAt(unsigned int);
	size_t strlenAt(unsigned int);
	size_t indexlenAt(unsigned int);
	const size_t &size;
	const size_t &max_len;
};

#endif // __SUBSTRINGS_H__
