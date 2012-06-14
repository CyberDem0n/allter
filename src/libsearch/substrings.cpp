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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <vector>
#include <iostream>

#include "substrings.h"
#include "charset.h"

CSubstrings::CSubstrings(const char *str) :
	_only_index(false), _substrings(NULL), length(strlen(str)), string(str),
			size(_size), max_len(_max_len) {
	getSubstrings();
}

CSubstrings::CSubstrings(const char *str, size_t length) :
	_only_index(true), _substrings(NULL), length(length), string(str),
			size(_size), max_len(_max_len) {
	getSubstrings();
}

CSubstrings::~CSubstrings() {
	if (_substrings) {
		if (!_only_index) delete [] _substrings_container;
		delete [] _indexes_container;
		delete [] _substrings;
	}
}

void CSubstrings::getSubstrings() {
	if (!good_chars_inited)
		init();

	size_t ipos = 0, upos = 0;

	_size = _max_len = 0;
	_max_len_id = -1;

	bool was_bad = true;
	wchar_t *wstring = new wchar_t[length + 1];
	wchar_t *p = wstring;
	size_t wlength = mbstowcs(wstring, string, length);
	if (wlength < 1)
		goto get_substr_ret;

	wstring[wlength++] = L'\0';

	mbstate_t ps;

	_substrings = new struct substrings [wlength/2];
	_indexes_container = new unsigned char [wlength ];
	if (!_only_index) {
		_substrings_container = new char [length + 1];
		memset(&ps, '\0', sizeof(ps));
	}

	while (true) {
		if ('\0' != good_chars[*p]) {
			if (was_bad) {
				_substrings[_size].index = _indexes_container + ipos;
				if (!_only_index) _substrings[_size].string = _substrings_container + upos;
				was_bad = false;
			}
			_indexes_container[ipos++] = good_chars[*p];
			if (!_only_index) {
				size_t t = wcrtomb(_substrings_container + upos, *p, &ps);
				if (t > 0) upos += t;
			}
		} else {
			if (!was_bad) {
				_substrings[_size].index_length = _indexes_container + ipos - _substrings[_size].index;
				if (_max_len < _substrings[_size].index_length) {
					_max_len = _substrings[_size].index_length;
					_max_len_id = _size;
				}
				_indexes_container[ipos++] = '\0';
				if (!_only_index) {
					_substrings[_size].length = _substrings_container + upos - _substrings[_size].string;
					_substrings_container[upos++] = '\0';
				}
				_size++; was_bad = true;
			}
		}
		if ('\0' == *(p++)) break;
	}

get_substr_ret:
	delete [] wstring;
}

const char *CSubstrings::stringAt(int n) {
	if (!_only_index && n < size)
		return _substrings[n].string;
	return NULL;
}

const unsigned char *CSubstrings::indexAt(int n) {
	if (n < size)
		return _substrings[n].index;
	return NULL;
}

size_t CSubstrings::strlenAt(int n) {
	if (!_only_index && n < size)
		return _substrings[n].length;
	return -1;
}

size_t CSubstrings::indexlenAt(int n) {
	if (n < size)
		return _substrings[n].index_length;
	return -1;
}

bool CSubstrings::initCharsetTable(const char *charset_table) {
	std::vector<RemapRange> remaps;
	CharsetDefinitionParser parser;
	int cnt = 1;
	if (!parser.parse(charset_table, remaps)) {
		std::cerr << "Error, " << parser.getLastError() << std::endl;
		return false;
	}

	memset(good_chars, 0, sizeof(good_chars));

	for (std::vector<RemapRange>::iterator it = remaps.begin(); it != remaps.end(); it++) {
		bool remap = it->start != it->remapStart;
		int size = it->end - it->start + 1;
		for (int i = 0; i < size; i++) {
			if (remap) {
				if (!good_chars[it->remapStart + i])
					good_chars[it->remapStart + i] = cnt++;
				good_chars[it->start + i] = good_chars[it->remapStart + i];
			} else {
				if (!good_chars[it->start + i])
					good_chars[it->start + i] = cnt++;
			}
			if (cnt > 255) {
				std::cerr << "Error, charset table can't contain more then 255 characters" << std::endl;
				return false;
			}
		}
	}
	return true;
}

void CSubstrings::init(void) {
	setlocale(LC_ALL,"");
	if (initCharsetTable("0..9, a..z->A..Z, U+430..U+44F->U+410..U+42F, U+451->U+401, ~, !, @, #, $, %, ^, &, (, ), -, +, _, =, ., ', ;, U+002C"))
		good_chars_inited = true;
}

bool CSubstrings::good_chars_inited = 0;
unsigned char CSubstrings::good_chars[65536] = {0,};
/*
int main() {
	CSubstrings::init();
	CSubstrings s("/DK?FJGH фыва KD*FJG DFKH|JGK/");
	printf("%d\n", s.size);
	for (int i = 0; i < s.size; i++)
		printf("%s\n", s.stringAt(i));
}
*/