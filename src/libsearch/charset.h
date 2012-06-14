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
#ifndef CHARSET_H_
#define CHARSET_H_

#include <stdio.h>
#include <stdarg.h>
#include <vector>

struct RemapRange {
	int start;
	int	end;
	int remapStart;

	RemapRange() : start(-1), end(-1), remapStart(-1) {}
	RemapRange(int iStart, int iEnd, int iRemapStart) : start(iStart), end(iEnd), remapStart(iRemapStart) {}
};

inline bool operator < (const RemapRange &a, const RemapRange &b) {
	return a.start < b.start;
}

class CharsetDefinitionParser {
private:
	bool errorb;
	char errstr[1024];
	const char *current;

	bool error(const char *formatstr, ...);
	void skipSpaces();
	bool isEof();
	bool checkEof();
	int hexDigit(int c);
	int parseCharsetCode();
	bool addRange(const RemapRange &range, std::vector<RemapRange> &ranges);

public:
	CharsetDefinitionParser() : errorb(false) {}
	bool parse(const char *str, std::vector<RemapRange> &ranges);
	const char *getLastError();
	virtual ~CharsetDefinitionParser() {}
};

#endif /* CHARSET_H_ */
