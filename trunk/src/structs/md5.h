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
#ifndef MD5_H_
#define MD5_H_

#include <dlfcn.h>
#include <stdint.h>

struct MD5Context {
	uint32_t buf[4];
	uint32_t bits[2];
	uint8_t in[64];
};

void *handle = RTLD_NEXT;
static void (*Md5Init)(struct MD5Context *context) = 0;
static void (*Md5Update)(struct MD5Context *context, const uint8_t *buf, size_t len) = 0;
static void (*Md5Final)(uint8_t digest[16], struct MD5Context *context) = 0;

void MD5Init(struct MD5Context *context) {
	if (NULL == Md5Init)
		*(void **)&Md5Init = dlsym(handle, "MD5Init");
	Md5Init(context);
}

void MD5Update(struct MD5Context *context, const uint8_t *buf, size_t len) {
	if (NULL == Md5Update)
		*(void **)&Md5Update = dlsym(handle, "MD5Update");
	Md5Update(context, buf, len);
}

void MD5Final(uint8_t digest[16], struct MD5Context *context) {
	if (NULL == Md5Final)
		*(void **)&Md5Final = dlsym(handle, "MD5Final");
	Md5Final(digest, context);
}

#endif /* MD5_H_ */
