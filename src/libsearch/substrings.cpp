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

unsigned char good_chars[256];
int good_chars_inited = 0;

CSubstrings::CSubstrings(unsigned char *str) {
	s = str;
	len = strlen((char *) str);
	wc = 0;
	max_len = 0;
	max_len_id = -1;
	getSubstrings();
}

CSubstrings::CSubstrings(unsigned char *str, unsigned int length) {
	s = str;
	len = length;
	wc = 0;
	max_len = 0;
	max_len_id = -1;
	getSubstrings();
}

CSubstrings::~CSubstrings() {
	if (substrings) {
		for (int i = 0; i < wc; i++)
			delete[] substrings[i];
		free(substrings);
	}
}

void CSubstrings::getSubstrings(void) {
	int i, prev = 0;

	if (s[len - 1] == '/')
		len--;

	if (!good_chars_inited) {
		memset(good_chars, 1, sizeof(good_chars));
		good_chars[(int) ' '] = 0;
		good_chars[(int) '*'] = 0;
		good_chars[(int) '?'] = 0;
	}

	substrings = (unsigned char **) malloc(((int) ((len + 1) / 2)) * sizeof(unsigned char *));

	for (i = 1; i < (int) len; i++)
		if (good_chars[s[i]] == 0 && good_chars[s[i - 1]]) {
			substrings[wc] = new unsigned char[i - prev + 1];
			memcpy(substrings[wc], s + prev, i - prev);
			substrings[wc++][i - prev] = '\0';
			if (max_len < i - prev) {
				max_len = i - prev;
				max_len_id = wc - 1;
			}
		} else if (good_chars[s[i]] && good_chars[s[i - 1]] == 0)
			prev = i;
	if (good_chars[s[len - 1]]) {
		substrings[wc] = new unsigned char[i - prev + 1];
		memcpy(substrings[wc], s + prev, i - prev);
		substrings[wc++][i - prev] = '\0';
		if (max_len < i - prev) {
			max_len = i - prev;
			max_len_id = wc - 1;
		}
	}
	substrings = (unsigned char **) realloc(substrings, wc * sizeof(unsigned char *));
}

unsigned char *CSubstrings::operator[](int n) {
	if (n < wc)
		return substrings[n];
	return (unsigned char *) NULL;
}

void CSubstrings::init(void) {
	char gdch[] = "QWERTYUIOPASDFGHJKLZXCVBNM1234567890êãõëåîçûýúèÿæù÷áðòïìäöüñþóíéôøâà³~!@#$%^&()-+_=.,';`";
	int ln = strlen(gdch);
	memset(&good_chars, 0, 256);
	for (int i = 0; i < ln; i++)
		good_chars[(unsigned char) gdch[i]] = 1;
	good_chars_inited = 1;
}
/*
 int main() {
	CSubstrings::init();
	CSubstrings s((unsigned char *) "DK?FJGH KD*FJG DFKH|JGK");
	printf("%d\n", s.wc);
	for (int i = 0; i < s.wc; i++)
		printf("%s\n", s[i]);
}
*/
