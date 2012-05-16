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
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "counter.h"

CCounter::CCounter() {
	updated = 0;
	ip_update_interval = 30;
	cur_time = time(NULL);
	today_count = (struct today_count *) NULL;
}

CCounter::~CCounter() {
	if (today_count)
		delete [] today_count;
}

void CCounter::OpenDB(void) {
	f = open(DB_FILE, O_RDWR);
	if (f != -1) {
		flock(f, LOCK_EX);
		GetData();
	} else {
		struct tm *lt = localtime(&cur_time);
		count.time = cur_time - lt->tm_hour * 3600 - lt->tm_min * 60 - lt->tm_sec;
		count.total = count.today = count.unique = 0;
		f = creat(DB_FILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (f)
			flock(f, LOCK_EX);
	}
}

void CCounter::CloseDB(void) {
	if (f != -1) {
		flock(f, LOCK_UN);
		close(f);
	}
}

void CCounter::Increment(char *ip) {
	unsigned int i, found = 0;
	ip_addr = inet_addr(ip);

	if (count.unique > 0) {
		today_count = new struct today_count[count.unique];
		int items = read(f, today_count, sizeof(struct today_count)	* count.unique);
		items /= sizeof(struct today_count);
		if (items < (int) count.unique)
			count.unique = items;
	} else {
		if (f != -1) {
			close(f);
			f = creat(DB_FILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
			if (f != -1)
				flock(f, LOCK_EX);
		}
	}

	if (f != -1) {
		for (i = 0; i < count.unique; i++)
			if (today_count[i].ip_addr == ip_addr) {
				found = 1;
				break;
			}

		if (found) {
			if (cur_time - today_count[i].time > ip_update_interval) {
				lseek(f, sizeof(struct count) + sizeof(struct today_count) * i,	SEEK_SET);
				count.total++;
				count.today++;
				Update();
			} else {
				count.total = today_count[i].total;
				count.today = today_count[i].today;
				count.unique = today_count[i].unique;
			}
		} else {
			lseek(f, sizeof(struct count) + sizeof(struct today_count) * count.unique, SEEK_SET);
			count.total++;
			count.today++;
			count.unique++;
			Update();
		}
	}
}

void CCounter::GetHTML(void) {
	printf("<TABLE CELLPADDING=0 CELLSPACING=0 BORDER=0><TR bgcolor=#EEEEEE>");
	printf("<td align=left><span class=copy>Total:</span> </td>");
	printf("<td align=right><span class=copy color=#FF0000>%d </span></td>", getTotal());
	printf("</TR><TR bgcolor=#FEFED0><td align=left><span class=copy>Hits today:</span> </td>");
	printf("<td align=right><span class=copy color=#FF0000>%d </span></td>", getToday());
	printf("</TR><TR bgcolor=#FEFED0><td align=left><span class=copy>Hosts today:</span> </td>");
	printf("<td align=right><span class=copy color=#FF0000>%d </span></td>", getUnique());
	printf("</TR></TABLE>");
}

void CCounter::PrintAll(void) {
	if (updated) {
		FILE *log;
		if ((log = fopen(DB_LOG_FILE, "ab"))) {
			flock(fileno(log), LOCK_EX);
			fprintf(log, "%s:%d %s", getenv("REMOTE_ADDR"), getTotal(), ctime(&cur_time));
			flock(fileno(log), LOCK_UN);
			fclose(log);
		}
	}
}

void CCounter::Update(void) {
	struct today_count temp;
	temp.ip_addr = ip_addr;
	temp.time = cur_time;
	temp.total = count.total;
	temp.today = count.today;
	temp.unique = count.unique;
	write(f, &temp, sizeof(struct today_count));
	lseek(f, 0, SEEK_SET);
	write(f, &count, sizeof(struct count));
	updated = 1;
}

void CCounter::GetData(void) {
	if (read(f, &count, sizeof(struct count)) != sizeof(struct count))
		count.time = count.total = 0;

	if (cur_time - count.time > 86400) {
		struct tm *lt = localtime(&cur_time);
		count.time = cur_time - lt->tm_hour * 3600 - lt->tm_min * 60 - lt->tm_sec;
		count.today = count.unique = 0;
	}
}

unsigned int CCounter::getTotal(void) {
	return count.total;
}

unsigned int CCounter::getToday(void) {
	return count.today;
}

unsigned int CCounter::getUnique(void) {
	return count.unique;
}
