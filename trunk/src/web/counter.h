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
#ifndef __COUNTER_H__
#define __COUNTER_H__
#include <time.h>

struct count {
	unsigned int time;
	unsigned int total;
	unsigned int today;
	unsigned int unique;
};

struct today_count {
	unsigned int ip_addr;
	unsigned int time;
	unsigned int total;
	unsigned int today;
	unsigned int unique;
};

class CCounter {
private:
	int f;
	time_t cur_time;
	unsigned int ip_addr;
	unsigned int ip_update_interval;
	struct count count;
	struct today_count *today_count;
	void GetData(void);
	void Update(void);
	int updated;
public:
	CCounter();
	~CCounter();
	void GetHTML(void);
	void PrintAll(void);
	void Increment(char *);
	void OpenDB(void);
	void CloseDB(void);
	unsigned int getTotal(void);
	unsigned int getToday(void);
	unsigned int getUnique(void);
};
#endif // __COUNTER_H__
