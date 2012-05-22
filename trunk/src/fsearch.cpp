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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <mysql.h>
#include <my_global.h>
#include <m_string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <locale.h>
#include <ctype.h>

#include "web/fsearch.h"
#include "libsearch/searcher.h"
#include "libsearch/substrings.h"
#include "web/counter.h"
#include "web/libcgi.h"
//#include "pingsv3.h"

#define MAX_BUF 4096
#define MAX_STR_LEN 512

//#define TMPL_PATH "/var/www/htdocs/tmpls/"
//#define PINGER_PATH "/home/allter/pinger/hosts/"
//#define LOG_PATH "/home/allter/logs/query/"

char *q = (char *)NULL;
char *query = (char *)NULL;
int type = 0;
long long max_size = -1;
long long min_size = -1;
int added = 0;
int upd = 0;
int bitrate = -1;
int query_type = 0;

MYSQL mysql;

char *index_get_file_ids(void)
{
		char *tmp,*ret = (char *)NULL;
		CSearcher s(INDEX_PATH "fls.txt");
		tmp = s.doQuery('0'+(char)type, q);
		if(!tmp) return (char *)NULL;
		ret = new char[strlen(tmp)+1];
		strcpy(ret,tmp);
		if (*tmp) delete [] tmp;
		return ret;
}

int check_query(char *s)
{
		if(s[0] == '/') return 4;
		while(*s) {
				if(*s=='*' || *s=='?') return 2;
				s++;
		}
		return 0;
}

void sql_get_search_results(const char *uids)
{
		char SQL_STRING[32768],*end;

		end = strmov(SQL_STRING,"SELECT cip,dnsname,shares.name,paths.name,files.name,size,type,paths2files.updated,p2f,shares.uid,add_info&16383,shares.updated FROM files STRAIGHT_JOIN paths2files STRAIGHT_JOIN paths STRAIGHT_JOIN shares STRAIGHT_JOIN hosts WHERE ");
		if(uids && strlen(uids)) {
				end = strmov(end, "files.uid IN(");
				end = strmov(end, uids);
				end = strmov(end, ") AND ");
		} else {
				if(check_query(q)) {
						end = strmov(end, "files.name LIKE '");
						end += mysql_real_escape_string(&mysql, end, query, strlen(query));
						end = strmov(end, "' AND ");
				} else {
						CSubstrings s(q);
						for(int i=0;i<s.size;i++) {
								end = strmov(end, "files.name LIKE '%");
								end += mysql_real_escape_string(&mysql, end, s.stringAt(i), s.strlenAt(i));
								end = strmov(end, "%' AND ");
						}
				}

				if(type == 1) end = strmov(end,"type!=2 AND ");
				else if(type) end += sprintf(end,"type=%d AND ",type);
				if(type!=2 && (max_size>-1 || min_size>-1 || bitrate>-1)) {
						if(max_size>-1 && max_size == min_size)
								end += sprintf(end,"size=%llu AND ",max_size);
						else {
								if(max_size>-1) end += sprintf(end,"size<=%llu AND ",max_size);
								if(min_size>-1) end += sprintf(end,"size>=%llu AND ",min_size);
						}
						if(bitrate>0 && (!type || type == 3 || type == 1)) end += sprintf(end,"add_info&16383>=%d AND ",bitrate);
				}
				if(upd) end += sprintf(end,"paths2files.updated>%u AND ",upd);
		}

		end = strmov(end,"file=files.uid AND path=paths.uid AND share=shares.uid AND host=hosts.uid ORDER BY files.name,type,size,shares.updated DESC LIMIT 700");
//		fprintf(stderr,"\n%s\n",SQL_STRING);
		mysql_real_query(&mysql,SQL_STRING,(unsigned int) (end - SQL_STRING));
}

char *last_updated(int time)
{
		static char ret[10];
		time = (int)((time-1)/3600) + 1;
		if(time < 24) sprintf(ret,"%dh",time);
		else sprintf(ret,"%dd",(int)(time/24));
		return ret;
}

int print_search_results()
{
//		int st;
		int at,i = 0;
		int nd = 0,ut,ct = time(NULL);
		char prev[4096],prevt[3],prevs[32];
		char path[4096];
		const char *proto_konq = "smb://";
		const char *proto_opera = "file://";
		const char *proto_fox = "file://///";
		const char *proto_oth = "file:////";
		const char *proto;
		int nix = 0;

		MYSQL_RES *res;
		MYSQL_ROW row;

		*prev = *prevs = *prevt = '\0';

		if(!(res = mysql_store_result(&mysql)) || !(row = mysql_fetch_row(res))) {
				print_file(TMPL_PATH "not_found.html");
				return 0;
		}
		mysql_data_seek(res,0);
		char *user_agent = getenv("HTTP_USER_AGENT");
		if(user_agent) {
			if (strstr(user_agent,"Konqueror") || strstr(user_agent,"Mac OS") || strstr(user_agent,"X11")) proto = proto_konq,nix=1;
			else if (strstr(user_agent,"Opera")) proto = proto_opera;
			else if (strstr(user_agent,"Firefox")) proto = proto_fox;
			else proto = proto_oth;
		} else proto = proto_oth;

		printf("<table width=100%% border=0 cellspacing=0 cellpadding=1 bgcolor=#000000 align=center>\n\
	<tr>\n\
	<td>\n\
		<table width=100%% border=0 cellspacing=0 cellpadding=6 bgcolor=#FFFFFF align=center>\n\
		<tr>\n\
		    <td><table border=0 cellspacing=0 cellpadding=0 width=588><tr><td>&nbsp;</td></tr></table></td>\n\
		</tr>\n\
			<tr>\n\
				<td align=left>\n");

//		read_ping_cfg();
//		load_ping_status();

        while((row = mysql_fetch_row(res))) {
				if(strcmp(row[4],prev) || strcmp(row[6],prevt) || strcmp(row[5],prevs)) {
						if(*prev) printf("</p>\n");
						nd=1;
						strcpy(prev,row[4]);strcpy(prevt,row[6]);strcpy(prevs,row[5]);
						printf("<p class=f><b>%d - %s</b> {%s}", ++i, row[4], row[5]);
						if(atol(row[10])) printf(" <font color=blue>%s</font> Kbps", row[10]);
						printf("</p>\n");
				}
				strcpy(path,row[3]);
				for(at=0;at<(int)strlen(path);at++)
						if(path[at] == '\\')
								path[at] = '/';
				ut = ct - atoi(row[11]);

				if(nd) printf("<p class=d>"), nd=0;
				else printf("<br>\n");
				
				printf("<a href=\"%s%s/%s%s\" title=\"Browse using IP [%s]\"",proto,row[0],row[2],path,row[0]);
				/*st = host_available(abcd_to_ip(row[0])); //et_status(row[0]);
				if(st == 0) printf(" class=n>[-]");
				else if(st == 1) printf(" class=p>[+]");
				else */printf(">[?]");
				printf("</a> (<a href=\"browse.cgi?");
				if(strcmp(row[3],"\\")) printf("dir=%s",row[8]);
				else printf("share=%s",row[9]);
				printf("\" title=\"Browse using AllTer samba browser\">%s</a>) ", last_updated(ut));
				
				if (nix) printf("<a href=\"%s%s/%s%s\" title=\"Browse using name\">//%s/%s%s</a>",proto,row[1],row[2],path,row[1],row[2],path);
				else printf("<a href=\"%s%s/%s%s\" title=\"Browse using name\">\\\\%s\\%s%s</a>",proto,row[1],row[2],path,row[1],row[2],row[3]);
				time_t added_t = atol(row[7]);
				printf(" Added: %s",ctime(&added_t));
		}
//		free_ping_status();
//		free_ping_cfg();

		mysql_free_result(res);
		printf("</p>\n");
		printf("</td>\n\
			</tr>\n\
		</table>\n\
	</td>\n\
	</tr>\n\
	</table>\n");
	return 1;
}

int check_query_type()
{
		int i,len,gc1 = 0;
		len = strlen(q);
		if(!len) return 0;
		for(i=0;i<len;i++)
				if(q[i] != '*' && q[i] != '?')
						gc1++;
				
		if(!gc1) return 0;
		if(gc1<3 || (gc1<3 && q[0] == '/')) return 1;
		if(q[0] == '/') return 2;
		int found = 0;
		CSubstrings s(q);
		for(i=0;i<s.size;i++)
				if(s.strlenAt(i) > 1) {found = 1; break; }
		if(!found) return 1;
		if(s.size==1 && s.strlenAt(0) == 2) return 1;

//		if(type>1 || max_size || min_size || bitrate || added) return 8;
		if(max_size>-1 || min_size>-1 || bitrate>-1 || added>0) return 8; // Because my searcher works more quickly then MySQL
		return 16;
}

void get_query_string() {
		int i,j=0;
		int beg=0;
		int k=0;
		int started = 0;
		char *newq = new char[strlen(q)+1];
		bzero(newq,strlen(q)+1);
		query = new char[strlen(q)+5];


		for(i=0;i<(int)strlen(q);i++) {
				if(q[i] == '%') q[i] = '*';
				if(q[i] == '\\') q[i] = '/';
				if(!started && ((q[0] == '*' && q[i]!='*' && q[i]!='?' && q[i]!='/') || q[i]!='*')) {
						newq[k++]= q[i];
						started = 1;
						continue;
				}
				if(started) {
						if(q[i] == '*') {
								if(newq[k-1] == '*') continue;
								if(newq[k-1] == '?') {
										newq[k-1] = '*';
										continue;
								}
						}
						if(q[i] == '?' && newq[k-1] == '*') continue;
						if(q[i] == '/' && i!=(int)strlen(q)-1) continue;
						newq[k++] = q[i];
				}
		}
		delete [] q;
		
		if(newq[0] == '/' && newq[1] == '*') q = newq+2;
		else q = newq;

		if(q[strlen(q)-1] == '*') q[strlen(q)-1] = '\0';
		
		if(q[0] == '/')	beg++;
		else query[j++] = '%';

		for(i=beg;i<(int)strlen(q);i++) {
				if(q[i] == '*') query[j++] = '%';
				else if(q[i] == '?') query[j++] = '_';
				else query[j++] = q[i];
		}
		if(q[strlen(q)-1] == '/') query[j-1] = '\0';
		else query[j++] = '%';
		query[j++] = '\0';
}

void mysql_error_message(void)
{
		printf("<h1>Can't connect to database!</h1>");
		print_footer(0);
}

int main(int argc, char **argv)
{
		FILE *f;
		struct timeval tb,te;
		struct timezone tz;
		double search_time=0,create_time;
		char *uids;
		CGI *cgi = new CGI();

		setlocale(LC_CTYPE,"ru_RU.KOI8-R");

		print_header();
		print_top_table();
		char *tmp = cgi->param("search=");
		if(tmp) {
				strcpy((q = new char[strlen(tmp)+1]), tmp);
				free(tmp);
				
				type = cgi->int_param("type=");
				if (type<0 || type>4) type=0;
				max_size = cgi->long_param("max_size=");
				min_size = cgi->long_param("min_size=");
				added = cgi->int_param("added=");
				upd = added>0?(time(NULL) - added):0;
				bitrate = cgi->int_param("min_bitrate=");
				get_query_string();
				query_type = check_query_type();

				if(query_type > 1) {
						mysql_init(&mysql);
						if(!mysql_real_connect(&mysql,DB_HOST,DB_USER,DB_PASSWD,DB_BASE,0,NULL,0))  {
								mysql_error_message();
								return 1;
						}
						mysql_query(&mysql, "SET NAMES KOI8R");
						gettimeofday(&tb,&tz);

						if(query_type > 15) {
								uids = index_get_file_ids();

								if(uids)
										if(strlen(uids)) sql_get_search_results(uids);
										else sql_get_search_results("-1");
								else sql_get_search_results((char *)NULL);
								delete [] uids;
						}
						else sql_get_search_results((char *)NULL);

						gettimeofday(&te,&tz);
						search_time = te.tv_sec+te.tv_usec/1000000.0-tb.tv_sec-tb.tv_usec/1000000.0;
				}
		}
		print_top_search();
		if(q && query_type > 1) {
				int have_results = print_search_results();
				gettimeofday(&te,&tz);
				create_time = te.tv_sec+te.tv_usec/1000000.0-tb.tv_sec-tb.tv_usec/1000000.0;
				printf("<br>Search time: %.3f seconds\n<br>\n",search_time);
				printf("This page created in %.3f seconds\n<br><br>",create_time);
				mysql_close(&mysql);
				if (have_results) print_bottom_search();
				if(strcmp(ADMIN_IP_ADDRESS, getenv("REMOTE_ADDR"))) {
						if((f = fopen(LOG_PATH "files_query.log","ab"))) {
								flock(fileno(f), LOCK_EX);
								fprintf(f,"%s:%d stime=>%.3f ctime=>%.3f type=>%d",getenv("REMOTE_ADDR"),(int)time(NULL),search_time,create_time,type);
								if(max_size>-1) fprintf(f," max_size=> %llu",max_size);
								if(min_size>-1) fprintf(f," min_size=> %llu",min_size);
								if(bitrate>-1) fprintf(f," bitrate=> %d",bitrate);
								fprintf(f," query=> %s\n",q);
								flock(fileno(f), LOCK_UN);
								fclose(f);
						}
				}
				printf("<br><center><table cellspacing=0 cellpadding=5 border=0><tr><td>");
		} else {
				printf("<center><table cellspacing=0 cellpadding=5 border=0><tr><td>");
				CCounter *count = new CCounter();
				count->OpenDB();
				if(getenv("REMOTE_ADDR") && strcmp(ADMIN_IP_ADDRESS, getenv("REMOTE_ADDR"))) {
						count->Increment(getenv("REMOTE_ADDR"));
						count->PrintAll();
				}
				count->CloseDB();
				count->GetHTML();
				delete count;
				printf("</td><td>");
		}
		print_stat_table();
		printf("</td><td>");
		print_copyright();
		printf("</td></tr></table></center>");
		if(q && query_type > 1) print_footer(1);
		else print_footer(0);
		delete cgi;
		return 0;
}
