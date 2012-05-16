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

#include "searcher.h"


#define MAX_STR_LEN 1024

CSearcher::CSearcher(const char *fname_base, char *q, char t)
{
	type = (unsigned char)t;
	query_len = strlen(q);
	query = new unsigned char[query_len+1];
	for(int i=0;i<query_len;i++)
		query[i] = (unsigned char)toupper((unsigned char)q[i]);
	query[query_len] = '\0';

	query_type = check_query(query);
	s = new CSubstrings(query);

	int len = strlen((base_name = fname_base))+5;
	strcpy((id2_name = new char[len]),fname_base); strcat(id2_name,".id2");
	strcpy((id4_name = new char[len]),fname_base); strcat(id4_name,".id4");
	strcpy((idx_name = new char[len]),fname_base); strcat(idx_name,".idx");

	/* max_results = 99; A5B !!! */
	max_results = 399;

	*ret = '\0'; done = 0;
}

CSearcher::~CSearcher()
{
	delete s;
	delete [] query;
	delete [] id2_name;
	delete [] id4_name;
	delete [] idx_name;
}

inline int CSearcher::mstrstr(unsigned char *buf)
{
	for(int j=0;j<s->wc;j++)
		if(!strstr((const char *)buf,(const char *)(*s)[j]))
			return 0;
	return 1;
}

inline int CSearcher::wstrstr(unsigned char *buf)
{
	unsigned char *b,*b1,*s,*sini;
	b=buf;
	s=sini=query;
	while(*b) {
		b1=b;
		while((*s==*b1 || *s=='?') && *s!='*' && *s) s++,b1++;
		if(!*s) return 1;
		if(*s=='*') s++,sini=s,b=b1;
		else s=sini,b++;
	}
	return 0;
}

inline int CSearcher::bstrstr(unsigned char *buf)
{
	unsigned char *b,*b1,*s,*sini;
	int wasstar=0, buflen=strlen((char *)buf);
	s=query+query_len-2;
	if (buf[buflen-1] == '\n') buflen--;
	b=buf+buflen-1;
	sini=s;
	while(b>=buf) {
		b1=b;
		while((*s==*b1 || *s=='?') && *s!='*' && s>=query) s--,b1--;
		if(s<query) return 1;
		if(*s=='*') s--,sini=s,b=b1,wasstar=1;
		else if(wasstar) s=sini,b--;
		else return 0;
	}
	return 0;
}

int CSearcher::check_query(unsigned char *str)
{
	if(str[strlen((char *)str)-1] == '/') return 4;
	while(*str) {
		if(*str=='*' || *str=='?') return 2;
		str++;
	}
	return 0;
}

int CSearcher::find_chain3(unsigned char *str, unsigned int *offset, unsigned int *size)
{
	int found = 0;
	unsigned int l = 0;
	unsigned int r = id4_len;
	unsigned int tmp = 0;
	unsigned short idx4 = (unsigned short)str[2];

	while (r-l > 1) { // �������� ����� ����� �������, � �������� ��������� ������ ����
		tmp = (unsigned int)((r+l)/2);
		if(id4[tmp].chr>>8==idx4) {found=1;break;}
		if(id4[tmp].chr>>8<idx4) l = tmp;
		else r = tmp;
	}
	if (!found) { // ��������� ������ � ��������� ��������
		if(id4[l].chr>>8==idx4) tmp = l;
		else if(id4[r].chr>>8==idx4) tmp = r;
		else return 0;
	}

	found = 0;
	for(int i=(int)tmp;i>=0;i--) // �������� ����� ������ �������, � �������� ��������� ������ ����
		if(id4[i].chr>>8!=idx4) {
			*offset = id4[i+1].offset;
			found = 1;
			break;
		}
	if(!found) *offset = id4[0].offset;

	found = 0;
	for(int i=(int)tmp;i<(int)id4_len;i++) // �������� ����� ��������� �������, � �������� ��������� ������ ����
		if(id4[i].chr>>8!=idx4) {
			*size = (id4[i].offset - *offset)>>2;
			return 1;
		}
	*size = ((id4[id4_len-1].offset - *offset)>>2)+id4[id4_len-1].size;
	return 1;
}

int CSearcher::find_chain4(unsigned char *str, unsigned int *offset, unsigned int *size)
{
	unsigned int l = 0;
	unsigned int r = id4_len;
	unsigned int tmp;
	unsigned short idx4 = (((unsigned short)str[2])<<8) | (unsigned short)str[3];

	while(r-l > 1) {
		tmp = (unsigned int)((r+l)/2);
		if(id4[tmp].chr==idx4) goto RET4;
		if(id4[tmp].chr<idx4) l = tmp;
		else r = tmp;
	}
	if(id4[l].chr==idx4) tmp = l;
	else if(id4[r].chr==idx4) tmp = r;
	else return 0;
RET4:
	*offset = id4[tmp].offset;
	*size = id4[tmp].size;
	return 1;
}

char *CSearcher::results(void)
{
	if(done) return ret;
	if(s->max_len < 3) return (char *)NULL;
	FILE *f;
	int idx_fd, id2_fd, id4_fd;
	char buf[MAX_STR_LEN];
	unsigned char *tmp;
	unsigned int offset, size, tmp_off, tmp_size, map_off, map_align;
	unsigned int i;
	unsigned int *offsets;
	int len, minus_len;
	char *tmp_id4 = NULL;
	int (CSearcher::*find_chain)(unsigned char *, unsigned int *, unsigned int *);
	int (CSearcher::*fullcheck)(unsigned char *);

	id4_fd = open(id4_name, O_RDONLY);
	if(id4_fd == -1) return (char *)NULL;

	offset = size = 0;

	id2_fd = open(id2_name, O_RDONLY);
	if(id2_fd == -1) {
		close(id4_fd);
		return (char *)NULL;
	}

	if(s->max_len > 3) {
		find_chain = &CSearcher::find_chain4;
		minus_len = 3;
	} else {
		find_chain = &CSearcher::find_chain3;
		minus_len = 2;
	}

	for(int i=0;i<s->wc;i++) {
		len = strlen((char *)(*s)[i]) - minus_len;
		for(int j=0;j<len;j++) {
			tmp_off = ((((unsigned int)(*s)[i][j])<<8) | ((unsigned int)(*s)[i][j+1]))*2*sizeof(unsigned int);
			if(lseek(id2_fd, tmp_off, SEEK_SET) == -1) {
				close(id2_fd);
				close(id4_fd);
				return (char *)NULL;
			}
			if (read(id2_fd, &map_off, 4)!=4);
			if (read(id2_fd, &id4_len, 4)!=4);
			if(id4_len) {
				if(lseek(id4_fd, 0, SEEK_END) < (off_t)(id4_len*sizeof(struct index4) + map_off)) {
					close(id2_fd);
					close(id4_fd);
					return (char *)NULL;
				}
				map_align = map_off%getpagesize();
				tmp_id4 = (char *)mmap(tmp_id4, id4_len*sizeof(struct index4)+map_align, PROT_READ, MAP_SHARED, id4_fd, map_off-map_align);
				id4 = (struct index4 *)(tmp_id4+map_align);
				if((*this.*find_chain)((unsigned char *)(*s)[i]+j,&tmp_off,&tmp_size)) {
					if(!size || size>tmp_size) {
						size = tmp_size;
						offset = tmp_off;
					}
				} else {
					munmap(tmp_id4, id4_len*sizeof(struct index4)+map_align);
					close(id2_fd);
					close(id4_fd);
					return (char *)""; // No results!!!!
				}
				munmap(tmp_id4, id4_len*sizeof(struct index4)+map_align);
			}
		}
	}
	close(id2_fd);
	close(id4_fd);

	if(!size) return (char *)""; // No results

	idx_fd = open(idx_name, O_RDONLY);
	if(idx_fd == -1) return (char *)NULL;

	if(lseek(idx_fd, 0, SEEK_END) < (off_t)(size*sizeof(unsigned int) + offset)) {
		close(idx_fd);
		return (char *)NULL;
	}

	tmp_id4 = NULL;

	map_align = offset%getpagesize();
	tmp_id4 = (char *)mmap(tmp_id4, size*sizeof(unsigned int)+map_align, PROT_READ, MAP_SHARED, idx_fd, offset-map_align);
	offsets = (unsigned int *)(tmp_id4+map_align);

	f = fopen(base_name ,"r");
	if(!f) return (char *)NULL;

	minus_len = len = 0;

	if(query_type==4)
		fullcheck = &CSearcher::bstrstr;
	else if(query_type==2)
		fullcheck = &CSearcher::wstrstr;
	else fullcheck = &CSearcher::mstrstr;

	for(i=0;i<size;i++) {
		if (fseek(f, offsets[i],0)==-1 || !fgets(buf, MAX_STR_LEN, f))
			continue;
		if ((tmp = (unsigned char *)strchr(buf,',')) == NULL)
			continue;
		if(!(type == '0' || (type == '1' && tmp[1] != '2') || tmp[1] == type))
			continue;
		if(!(*this.*fullcheck)(tmp+3))
			continue;
		memcpy(ret+minus_len, buf, tmp-(unsigned char *)buf);
		minus_len += (tmp-(unsigned char *)buf);
		ret[minus_len++] = ',';
		if(++len>max_results)
			break;
	}

	munmap(tmp_id4, size*sizeof(unsigned int)+map_align);
	close(idx_fd);
	fclose(f);

	if(minus_len) ret[minus_len-1] = '\0';

	done = 1;
	return ret;
}
