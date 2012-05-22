prefix		:= /home/allter
wwwroot		:= $(prefix)/www

FSEARCH_DEFINES	:= -DTMPL_PATH=\"$(wwwroot)/tmpls/\" -DLOG_PATH=\"$(prefix)/logs/query/\" -DINDEX_PATH=\"$(prefix)/index/\" -DADMIN_IP_ADDRESS=\"127.0.0.1\"
COUNTER_DEFINES	:= -DDB_FILE=\"$(prefix)/logs/count/counter.dat\" -DDB_LOG_FILE=\"$(prefix)/logs/count/all.txt\"
MYSQL_DEFINES	:= -DDB_HOST=\"localhost\" -DDB_USER=\"root\" -DDB_PASSWD=\"\" -DDB_BASE=\"allter\"
PINGER_DEFINES	:= -DPING_FILES_CFG=\"$(prefix)/pinger/ping_files.cfg\"

programs	:= crawler indexer searcher fsearch.cgi


compile_flags	:= -Wall -ggdb -MD -pipe `mysql_config --cflags`
link_flags	:= -ggdb -pipe
mysql_ldflags	:= `mysql_config --libs`

libsmbclient = $(shell ls /usr/lib*/libsmbclient.a 2> /dev/null | head -n 1)
ifneq ($(libsmbclient),)
	compile_flags := $(compile_flags) -DHAVE_STATIC_LIBSMBCLIENT
endif

db_dir		:= db
web_dir		:= web
libsearch_dir	:= libsearch

crawler_subdirs	:= client structs libs
crawler_objs	:= crawler.cpp $(wildcard $(addsuffix /*.c*, $(crawler_subdirs) $(db_dir)))
crawler_objs	:= $(crawler_objs:.cpp=.o)
crawler_objs	:= $(crawler_objs:.c=.o)

indexer_objs	:= indexer.cpp $(wildcard $(addsuffix /*.cpp, $(libsearch_dir)))
indexer_objs	:= $(indexer_objs:.cpp=.o)

searcher_objs	:= searcher.cpp $(wildcard $(addsuffix /*.cpp, $(libsearch_dir)))
searcher_objs	:= $(searcher_objs:.cpp=.o)

fsearch_objs	:= fsearch.cpp $(wildcard $(addsuffix /*.cpp, $(libsearch_dir) $(db_dir) $(web_dir)))
fsearch_objs	:= $(fsearch_objs:.cpp=.o)

source_dirs	:= . $(db_dir) $(libsearch_dir) $(web_dir) $(crawler_subdirs)
 
all: $(programs)
 
crawler: $(crawler_objs)
	g++ -o $@ $(crawler_objs) $(libsmbclient) $(link_flags) -lsmbclient -ltdb $(mysql_ldflags)
 
indexer: $(indexer_objs)
	g++ -o $@ $(indexer_objs) $(link_flags)

searcher: $(searcher_objs)
	g++ -o $@ $(searcher_objs) $(link_flags)

fsearch.cgi: $(fsearch_objs)
	g++ -o $@ $(fsearch_objs) $(link_flags) $(mysql_ldflags)

%.o: %.cpp
	g++ -o $@ -c $< $(compile_flags)

%.o: %.c
	g++ -o $@ -c $< $(compile_flags)

web/counter.o:
	g++ -o $@ -c web/counter.cpp $(compile_flags) $(COUNTER_DEFINES)

db/mydb.o:
	g++ -o $@ -c db/mydb.cpp $(compile_flags) $(MYSQL_DEFINES)

fsearch.o:
	g++ -o $@ -c fsearch.cpp $(compile_flags) $(MYSQL_DEFINES) $(FSEARCH_DEFINES)

.PHONY : clean

clean :
	rm -rf $(programs) $(wildcard $(addsuffix /*.d, $(source_dirs))) $(wildcard $(addsuffix /*.o, $(source_dirs)))

include $(wildcard $(addsuffix /*.d, $(source_dirs)))
