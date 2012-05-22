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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <byteswap.h>

#include "searcher.h"

#define MAX_STR_LEN 1024

CSearcher::CSearcher(const char *fname_base) :
	_id2(NULL), _id4(NULL), base_name(fname_base), max_results(399) {
	int len = strlen(fname_base) + 5;
	strcpy((id2_name = new char[len]), fname_base); strcat(id2_name, ".id2");
	strcpy((id4_name = new char[len]), fname_base); strcat(id4_name, ".id4");
	strcpy((idx_name = new char[len]), fname_base); strcat(idx_name, ".idx");

	mapIndexes();
}

CSearcher::~CSearcher() {
	delete [] id2_name;
	delete [] id4_name;
	delete [] idx_name;

	unmapIndexes();
}

void CSearcher::unmapIndexes() {
	if (NULL != _id2) {
		munmap(_id2, _id2_size * sizeof(struct index2));
		_id2 = NULL;
	}

	if (NULL != _id4) {
		munmap(_id4, _id4_size * sizeof(struct index4));
		_id4 = NULL;
	}
}

void *CSearcher::mapFile(const char *name, off_t offset, size_t &size, size_t &align) {
	int fd = open(name, O_RDONLY);
	if (fd == -1) return MAP_FAILED;
	off_t pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
	size_t filesize = lseek(fd, 0, SEEK_END);

	if (offset + size > filesize) {
		close(fd);
		return MAP_FAILED;
	}

	if (offset == 0 && size == 0)
		size = filesize;

	align = offset - pa_offset;

	void *ret = mmap(NULL, size + align, PROT_READ, MAP_SHARED, fd, pa_offset);
	close(fd);
	return ret;
}

bool CSearcher::mapIndexes() {
	size_t size = 0, align;
	void *map = mapFile(id2_name, 0, size, align);
	if (map == MAP_FAILED) return false;
	_id2_size = size/sizeof(struct index2);
	_id2 = (struct index2 *)map;

	size = 0;
	map = mapFile(id4_name, 0, size, align);
	if (map == MAP_FAILED) {
		unmapIndexes();
		return false;
	}
	_id4_size = size/sizeof(struct index4);
	_id4 = (struct index4 *)map;
	return true;
}

bool CSearcher::remapIndexes() {
	unmapIndexes();
	return mapIndexes();
}

bool CSearcher::mstrstr(const char *buf, CSubstrings &query) {
	for (int j=0; j<query.size; j++)
		if (NULL == strstr(buf, query.stringAt(j)))
			return false;
	return true;
}

bool CSearcher::wstrstr(const char *buf, CSubstrings &query) {
	const char *b1, *s, *sini;
	const char *b = buf;
	s = sini = query.string;
	while (*b) {
		b1 = b;
		while ((*s==*b1 || *s=='?') && *s!='*' && *s) s++, b1++;
		if (!*s) return true;
		if (*s=='*') s++,sini=s, b=b1;
		else s=sini, b++;
	}
	return false;
}

bool CSearcher::bstrstr(const char *buf, CSubstrings &query) {
	const char *q = query.string;
	const char *b, *b1, *s, *sini;
	int wasstar=0, buflen = strlen(buf);
	s = q + query.length - 2;

	while (buf[buflen-1] == '\n' || buf[buflen-1] == '\r')
		buflen--;

	b = buf + buflen - 1;
	sini = s;
	while (b>=buf) {
		b1 = b;
		while ((*s==*b1 || *s=='?') && *s!='*' && s >= q) s--, b1--;
		if (s < q) return true;
		if (*s=='*') s--, sini=s, b=b1, wasstar=1;
		else if (wasstar) s=sini, b--;
		else return false;
	}
	return false;
}

int CSearcher::check_query(const char *str) {
	if (str[strlen(str)-1] == '/') return 4;
	while (*str) {
		if (*str=='*' || *str=='?') return 2;
		str++;
	}
	return 0;
}

bool CSearcher::find_chain3(const unsigned char *str, unsigned int start, unsigned int end, unsigned int &offset, unsigned int &size) {
	bool found = false;
	unsigned int tmp, l = start, r = end;
	unsigned short idx4 = (unsigned short)str[2];

	while (r-l > 1) {
		tmp = (unsigned int)((r+l)/2);
		if ((_id4[tmp].chr>>8) == idx4) {found=true;break;}
		if ((_id4[tmp].chr>>8) < idx4) l = tmp;
		else r = tmp;
	}
	if (!found) {
		if ((_id4[l].chr>>8) == idx4) tmp = l;
		else if ((_id4[r].chr>>8) == idx4) tmp = r;
		else return false;
	}

	found = false;
	for (unsigned int i=tmp; i>=start; i--)
		if ((_id4[i].chr>>8) != idx4) {
			offset = _id4[i+1].offset;
			found = true;
			break;
		}
	if (!found) offset = _id4[start].offset;

	for (unsigned int i=tmp; i<end; i++)
		if ((_id4[i].chr>>8) != idx4) {
			size = (_id4[i].offset - offset)>>2;
			return true;
		}
	size = ((_id4[end-1].offset - offset)>>2)+_id4[end-1].size;
	return true;
}

bool CSearcher::find_chain4(const unsigned char *str, unsigned int start, unsigned int end, unsigned int &offset, unsigned int &size) {
	unsigned int tmp, l = start, r = end;
	unsigned short idx4 = (((unsigned short)str[2])<<8) | (unsigned short)str[3];

	while (r-l > 1) {
		tmp = (unsigned int)((r+l)/2);
		if (_id4[tmp].chr == idx4) goto RET4;
		if (_id4[tmp].chr < idx4) l = tmp;
		else r = tmp;
	}
	if (_id4[l].chr == idx4) tmp = l;
	else if (_id4[r].chr == idx4) tmp = r;
	else return false;
RET4:
	offset = _id4[tmp].offset;
	size = _id4[tmp].size;
	return true;
}

char *CSearcher::doQuery(const char type, const char *query) {
	CSubstrings s(query);
	if (s.max_len < 3) return (char *)NULL;

	bool (CSearcher::*fullcheck)(const char *, CSubstrings &);
	int query_type = check_query(query);
	if (query_type==4)
		fullcheck = &CSearcher::bstrstr;
	else if (query_type==2)
		fullcheck = &CSearcher::wstrstr;
	else fullcheck = &CSearcher::mstrstr;

	int minus_len;
	bool (CSearcher::*find_chain)(const unsigned char *, unsigned int, unsigned int, unsigned int &, unsigned int &);
	if (s.max_len > 3) {
		find_chain = &CSearcher::find_chain4;
		minus_len = 3;
	} else {
		find_chain = &CSearcher::find_chain3;
		minus_len = 2;
	}

	off_t offset = 0;
	size_t size = 0;
	for (int i=0; i<s.size; i++) {
		const unsigned char *str = (const unsigned char *)s.stringAt(i);
		int len = s.strlenAt(i) - minus_len;

		for (int j=0; j<len; j++) {
			unsigned short id2_pos = bswap_16(*((unsigned short *)(str + j)));
			if (id2_pos >= _id2_size) return (char *)NULL;

			struct index2 *id2 = _id2 + id2_pos;
			if (id2->id4_size < 1) return (char *)"";

			unsigned int id4_pos = id2->id4_offset/sizeof(struct index4);
			if (id4_pos >= _id4_size) return (char *)NULL;

			unsigned int tmp_off, tmp_size;
			if (!(*this.*find_chain)(str + j, id4_pos, id4_pos+id2->id4_size, tmp_off, tmp_size))
				return (char *)""; // No results

			if (!size || size > tmp_size) {
				size = tmp_size;
				offset = tmp_off;
			}
		}
	}

	if (size < 1) return (char *)""; // No results

	size_t align;
	void *idx = mapFile(idx_name, offset, size, align);
	if (MAP_FAILED == idx) return (char *)NULL;

	unsigned int *offsets = (unsigned int *)((char *)idx + align);

	FILE *f = fopen(base_name ,"r");
	if (!f) {
		munmap(idx, size + align);
		return (char *)NULL;
	}

	char buf[MAX_STR_LEN];
	int ret_len = 0, results = 0;
	char *ret = new char [max_results * 12];
	*ret = '\0';
	for (unsigned int i=0; i<size; i++) {
		if (fseek(f, offsets[i], SEEK_SET) == -1 || !fgets(buf, MAX_STR_LEN, f))
			continue;

		char *tmp = strchr(buf, ',');
		if (tmp == NULL || (!(type == '0' || (type == '1' && tmp[1] != '2')
				|| tmp[1] == type)) || !(*this.*fullcheck)(tmp + 3, s))
			continue;

		memcpy(ret+ret_len, buf, tmp - buf);
		ret_len += tmp - buf;
		ret[ret_len++] = ',';
		if (++results > max_results)
			break;
	}

	munmap(idx, size + align);
	fclose(f);

	if (!ret_len) {
		delete [] ret;
		return (char *)"";
	}

	ret[ret_len-1] = '\0';
	return ret;
}
