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
#include "substrings.h"

#define IS_GOOD(c) (good_chars[(unsigned char)c] != 0)
#define IS_BAD(c) (good_chars[(unsigned char)c] == 0)

CSubstrings::CSubstrings(const char *str) :
	_substrings(NULL), length(strlen(str)), string(str),
			size(_size), max_len(_max_len) {
	getSubstrings();
}

CSubstrings::CSubstrings(const char *str, size_t length) :
	_substrings(NULL), length(length), string(str),
			size(_size), max_len(_max_len) {
	getSubstrings();
}

CSubstrings::~CSubstrings() {
	if (_substrings) {
		delete [] _substrings_container;
		delete [] _substrings;
	}
}

void CSubstrings::getSubstrings(void) {
	if (!good_chars_inited) {
		memset(good_chars, 1, sizeof(good_chars));
		good_chars[(int) ' '] = 0;
		good_chars[(int) '*'] = 0;
		good_chars[(int) '?'] = 0;
	}

	size_t i, prev = 0, pos = 0;

	_size = _max_len = 0;
	_max_len_id = -1;

	_substrings = new struct substrings [(length + 1)/2];
	_substrings_container = new char [length + 1];

	for (i = 1; i < length; i++) {
		if (IS_GOOD(string[i - 1]) && (IS_BAD(string[i]) || i == length -1)) {
			size_t size = i - prev;
			if (i == length - 1 && IS_GOOD(string[i])) size++;

			if (_max_len < size) {
				_max_len = size;
				_max_len_id = _size;
			}

			memcpy(_substrings_container + pos, string + prev, size);
			_substrings_container[pos + size] = '\0';

			_substrings[_size].string = _substrings_container + pos;
			_substrings[_size++].length = size;

			pos += size + 1;
		} else if (IS_GOOD(string[i]) && IS_BAD(string[i - 1]))
			prev = i;
	}
}

const char *CSubstrings::stringAt(int n) {
	if (n < size)
		return _substrings[n].string;
	return NULL;
}

size_t CSubstrings::strlenAt(int n) {
	if (n < size)
		return _substrings[n].length;
	return -1;
}

void CSubstrings::init(void) {
	char gdch[] = "~!@#$%^&()-+_=.,';`";
	unsigned int i;

	memset(good_chars, 0, sizeof(good_chars));

	for (i = 0; i < sizeof(gdch)-1; i++)
		good_chars[(unsigned char)gdch[i]] = 1;
	for (i = (int)'0'; i <= (int)'9'; i++) // Digits
		good_chars[i] = 1;
	for (i = (int)'A'; i <= (int)'Z'; i++) // Latin capitals
		good_chars[i] = 1;
	for (i = 0xC0; i<= 0xDF; i++) // Russian capitals except Yo
		good_chars[i] = 1;
	good_chars[0xA8] = 1; // Russian capital Yo
	good_chars_inited = 1;
}

int CSubstrings::good_chars_inited = 0;
char CSubstrings::good_chars[256] = {0,};
/*
int main() {
	CSubstrings::init();
	CSubstrings s("/DK?FJGH KD*FJG DFKH|JGK/");
	printf("%d\n", s.size);
	for (int i = 0; i < s.size; i++)
		printf("%s\n", s.stringAt(i));
}
*/
