#Macros
INCLUDE = -I .
LIBPATH = -lpthread -lm -lrt -lcrypto -lssl -lidn 

CFLAGS = -g -O0 ${INCLUDE}
LIBS = libetdb.a
EXES = etdb etdb_cli

#Actions
all : clean $(LIBS) $(EXES)

clean : 
	rm -r -f *.o $(LIB) $(EXES)

libetdb.a: etdb_time.o etdb_alloc.o etdb_palloc.o etdb_trie.o etdb_log.o etdb_config.o etdb_string.o etdb_geo_hash.o etdb_stack.o etdb_event.o etdb_connection.o etdb_serv.o etdb_modules.o etdb_buf.o etdb_resp.o etdb_serv.o linenoise.o 
	ar cr $@ $^

etdb: etdb_server.o
	cc $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)

etdb_cli: etdb_client.o
	cc $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)

.o : .c
	cc -c $^ $(CFLAGS) -o $@
