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
#define ISCTRL(c)	((unsigned char)(c) < (unsigned char)' ')

extern char *q;
extern int type, added, bitrate;
extern int64_t min_size, max_size;

void print_encoded_string(char *str)
{
		for(;*str;str++) {
				switch(*str) {
						case '<':
								printf("&lt;");
								continue;
						case '>':
								printf("&gt;");
								continue;
						case '&':
								printf("&amp;");
								continue;
						case '"':
								printf("&quot;");
								continue;
						default:
								if ( !ISCTRL(*str) ) {
										putc(*str, stdout);
										continue;
								}
				}
				printf("&#%d;", (int)(unsigned char)*str);
		}
}

void print_header()
{
		printf("Content-type: text/html; charset=koi8-r\n\n");
		printf("<html>\n\
	<head>\n\
		<title>AllTer:-> File Search</title>\n\
		<link href=\"alter.css\" type=\"text/css\" rel=StyleSheet>\n\
		<meta http-equiv=Content-Type content=\"text/html; charset=koi8-r\">\n\
		<link rel=\"search\" type=\"application/opensearchdescription+xml\" title=\"Allter File search\" href=\"/opensearch.xml\" />\n\
	</head>\n\
<body bgcolor=#ffffff text=#000000>");
}

void print_file(const char *filename)
{
		int in_fd = open(filename, O_RDONLY);
		int len = lseek(in_fd, 0, SEEK_END);
		char *tmp = new char[len];
		lseek(in_fd, 0, SEEK_SET);
		if (read(in_fd, tmp, len*sizeof(char))==len*sizeof(char)) {
			fflush(stdout);
			if (write(STDOUT_FILENO, tmp, len*sizeof(char))==len*sizeof(char)) {}
		}
		close(in_fd);
}

void print_top_table()
{
		print_file(TMPL_PATH "top_table.html");
}

void print_stat_table()
{
		print_file(TMPL_PATH "stat_table.html");
}

void print_copyright()
{
		print_file(TMPL_PATH "copyright.html");
}

void print_footer(int a)
{
		printf("<script language=JavaScript>\n\
	document.forms[0].search.focus();\n\
	</script>\n");
	if (a) printf("<script language=JavaScript src=\"n.js\"></script>\n");
	printf("</body>\n\
</html>");
}

void print_search_form()
{
		printf("<form onsubmit=\"this.subm.disabled=true;\"><tr>\n\
								<td align=right>\n");
		if(q) {
				printf("<input type=text class=text size=35 name=search value=\"");
				print_encoded_string(q);
				printf("\">&nbsp;\n");
		}
		else printf("<input type=text class=text size=35 name=search>&nbsp;\n");
		printf("							<input class=button type=submit name=subm value=search>\n\
								</td>\n\
								<td>&nbsp;</td>\n\
							</tr>\n\
							<tr><td colspan=2>&nbsp;</td></tr>\n\
							<tr>\n\
								<td align=right class=name>\n\
									Looking for <select name=type>\n");
		if(type == 0) printf("<option value=0 selected>All</option>\n");
		else printf("<option value=0>All</option>\n");
		if(type == 1) printf("<option value=1 selected>Files</option>\n");
		else printf("<option value=1>Files</option>\n");
		if(type == 2) printf("<option value=2 selected>Folders</option>\n");
		else printf("<option value=2>Folders</option>\n");
		if(type == 3) printf("<option value=3 selected>Music</option>\n");
		else printf("<option value=3>Music</option>\n");
		if(type == 4) printf("<option value=4 selected>Video</option>\n");
		else printf("<option value=4>Video</option>\n");
		printf("</select>, added in the last\n<select name=added>\n<option value=0>---</option>\n");
		if(added == 86400) printf("<option value=86400 selected>Day</option>\n");
		else printf("<option value=86400>Day</option>\n");
		if(added == 604800) printf("<option value=604800 selected>Week</option>\n");
		else printf("<option value=604800>Week</option>\n");
		if(added == 2592000) printf("<option value=2592000 selected>Month</option>\n");
		else printf("<option value=2592000>Month</option>\n");
		printf("</select>\n\
								</td>\n\
								<td>&nbsp;</td>\n\
							</tr>\n\
							<tr><td colspan=2>&nbsp;</td></tr>\n\
							<tr>\n\
								<td align=right>\n\
									<span class=name>and it's size from </span>\n");
		if(min_size>-1) printf("<input type=text size=10 name=min_size value=\"%lld\">\n", (long long)min_size);
		else printf("<input type=text size=10 name=min_size>\n");
		printf("<span class=name>up to </span>\n");
		if(max_size>-1) printf("<input type=text size=10 name=max_size value=\"%lld\">\n", (long long)max_size);
		else printf("<input type=text size=10 name=max_size>\n");
		printf("<span class=name>bytes</span>\n\
									<br>\n\
								</td>\n\
								<td>&nbsp;</td>\n\
							</tr>\n\
							<tr><td colspan=2>&nbsp;</td></tr>\n\
							<tr>\n\
								<td align=right>\n");
		if(bitrate>-1) printf("<span class=name>mp3 bitrate more or equal <input type=text name=min_bitrate size=5 value=\"%d\">\n",bitrate);
		else printf("<span class=name>mp3 bitrate more or equal <input type=text name=min_bitrate size=5>\n");
		printf("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
								</td>\n\
								<td>&nbsp;</td>\n\
							</tr></form>\n");
}

void print_top_search()
{
		printf("<br>\n\
			<table width=100%% border=0 cellspacing=0 cellpadding=1 bgcolor=#000000 align=center>\n\
			<tr>\n\
				<td>\n\
				<table width=100%% border=0 cellspacing=0 cellpadding=0 bgcolor=#FFFFFF align=center>\n\
				<tr>\n\
					<td align=center>\n\
						<table width=600 border=0 cellspacing=0 cellpadding=0 bgcolor=#638cb6>\n\
							<tr>\n\
								<td width=80%% align=left class=name style=\"font-size: 16px;\">\n\
									&nbsp;&nbsp; AllTer file Searcher\n\
								</td>\n\
								<td width=20%%>&nbsp;</td>\n\
							</tr>\n");
		print_search_form();
		printf("<tr><td colspan=2>&nbsp;</td></tr>\n\
						</table>\n\
					</td>\n\
				</tr>\n\
				</table>\n\
				</td>\n\
			</tr>\n\
			</table>\n\
		<br>\n");
}

void print_bottom_search()
{
		printf("<table width=600 border=0 cellspacing=0 cellpadding=1 bgcolor=#000000 align=center>\n\
				<tr>\n\
					<td>\n\
						<table width=100%% border=0 cellspacing=0 cellpadding=0 bgcolor=#638cb6>\n\
							<tr>\n\
								<td width=80%%>&nbsp;</td>\n\
								<td width=20%%>&nbsp;</td>\n\
							</tr>\n");
		print_search_form();
		printf("<tr><td colspan=2 align=right class=name style=\"font-size: 16px;\">AllTer file Searcher&nbsp;&nbsp;</td></tr>\n\
						</table>\n\
					</td>\n\
				</tr>\n\
			</table>\n");
}

