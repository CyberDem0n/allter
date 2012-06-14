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
#include <ctype.h>

#include <algorithm>

#include "charset.h"

const char *CharsetDefinitionParser::getLastError() {
	return errorb?errstr:NULL;
}

bool CharsetDefinitionParser::isEof() {
	return ('\0' == *current);
}

bool CharsetDefinitionParser::checkEof() {
	if (!isEof())
		return false;
	error("unexpected end of line");
	return true;
}

bool CharsetDefinitionParser::error(const char *formatstr, ...) {
	va_list ap;
	va_start(ap, formatstr);
	int size = vsnprintf(errstr, sizeof(errstr), formatstr, ap);
	if ((ssize_t)size < sizeof(errstr)) {
		char sErrorBuffer[32];
		strncpy ( sErrorBuffer, current, sizeof(sErrorBuffer) );
		sErrorBuffer [ sizeof(sErrorBuffer)-1 ] = '\0';
		snprintf(errstr + size, sizeof(errstr) - size, " near '%s'", sErrorBuffer);
	}

	errstr[sizeof(errstr) - 1] = '\0';

	errorb = true;
	return false;
}

int CharsetDefinitionParser::hexDigit(int c) {
	if (c>='0' && c<='9') return c-'0';
	if (c>='a' && c<='f') return c-'a'+10;
	if (c>='A' && c<='F') return c-'A'+10;
	return 0;
}

void CharsetDefinitionParser::skipSpaces() {
	while (*current && isspace((unsigned char)*current))
		current++;
}

int CharsetDefinitionParser::parseCharsetCode() {
	int code = 0;
	const char *p = current;

	if (p[0]=='U' && p[1]=='+') {
		p += 2;
		while (isxdigit(*p))
			code = code*16 + hexDigit (*p++);

		while (isspace(*p))
			p++;
	} else {
		if ((*(unsigned char *)p)<32 || (*(unsigned char *)p)>127) {
			error("non-ASCII characters not allowed, use 'U+00AB' syntax");
			return -1;
		}

		code = *p++;
		while (isspace(*p))
			p++;
	}

	current = p;
	return code;
}

bool CharsetDefinitionParser::addRange(const RemapRange &range, std::vector<RemapRange> &ranges) {
	if (range.remapStart >= 0x20) {
		ranges.push_back(range);
		return true;
	}

	return error("dest range (U+%x) below U+20, not allowed", range.remapStart);
}


bool CharsetDefinitionParser::parse(const char *str, std::vector<RemapRange> &ranges ) {
	current = str;
	ranges.clear();

	while (*current) {
		skipSpaces();
		if (isEof()) break;

		if (',' == *current)
			return error("stray ',' not allowed, use 'U+002C' instead");

		const char *pStart = current;
		int startCode = parseCharsetCode();
		if (startCode < 0) return false;

		if (!*current || ',' == *current) {
			if (!addRange(RemapRange(startCode, startCode, startCode), ranges))
				return false;

			if (isEof()) break;
			current++;
			continue;
		}

		if ('-' == current[0] && '>' == current[1]) {
			current += 2;
			int destCode = parseCharsetCode();
			if (destCode < 0) return false;

			if (!addRange(RemapRange(startCode, startCode, destCode), ranges))
				return false;

			if (*current && *current++ != ',')
					return error("syntax error");
			continue;
		}

		if ('.' != current[0] || '.' != current[1])
			return error("syntax error");
		current += 2;

		skipSpaces();
		if (checkEof()) return false;

		int endCode = parseCharsetCode();
		if (endCode<0) return false;

		if (startCode > endCode) {
			current = pStart;
			return error("range end less than range start");
		}

		if (!*current || *current == ',') {
			if (!addRange(RemapRange(startCode, endCode, startCode), ranges))
				return false;

			if (isEof()) break;
			current++;
			continue;
		}

		if ('/' == current[0] && '2' == current[1]) {
			for (int i = startCode; i < endCode; i += 2 ) {
				if (!addRange(RemapRange(i, i, i+1), ranges))
					return false;
				if (!addRange(RemapRange(i+1, i+1, i+1), ranges))
					return false;
			}

			current += 2;
			skipSpaces();
			if (*current && *current++ != ',')
					return error("expected end of line or ','");
			continue;
		}

		if ('-' != current[0] || '>' != current[1])
			return error("expected end of line, ',' or '-><char>'");
		current += 2;

		skipSpaces();
		if (checkEof()) return false;

		const char * pRemapStart = current;
		int remapStartCode = parseCharsetCode();
		if (remapStartCode < 0)
			return false;

		if (checkEof()) return false;

		if ('.' != current[0] || '.' != current[1])
			return error("expected '..'");
		current += 2;

		int remapEndCode = parseCharsetCode();
		if (remapEndCode < 0) return false;

		if (remapStartCode > remapEndCode) {
			current = pRemapStart;
			return error("dest range end less than dest range start");
		}

		if ((remapEndCode - remapStartCode) != (endCode - startCode)) {
			current = pStart;
			return error("dest range length must match src range length");
		}

		if (!addRange(RemapRange(startCode, endCode, remapStartCode), ranges))
			return false;

		if (isEof()) break;

		if (',' != *current)
			return error("expected ','");
		current++;
	}

	std::sort(ranges.begin(), ranges.end());

	for (std::vector<RemapRange>::iterator it0 = ranges.begin(); it0 != ranges.end(); it0++) {
		std::vector<RemapRange>::iterator it1 = it0 + 1;
		if (it1 == ranges.end()) break;
		if (it0->end >= it1->start) {
			it0->end = it0->end>it1->end?it0->end:it1->end;
			ranges.erase(it1);
			it0--;
		}
	}

	return true;
}
