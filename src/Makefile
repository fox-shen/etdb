#Macros
INCLUDE = -I .
LIBPATH = -lpthread -lm -lrt -lcrypto -lssl -lidn 

CFLAGS = -g -O0 ${INCLUDE}
LIBS = libemdb.a
EXES = emdb emdb_cli

#Actions
all : clean $(LIBS) $(EXES)

clean : 
	rm -r -f *.o $(LIB) $(EXES)

libemdb.a: emdb_time.o emdb_alloc.o emdb_palloc.o emdb_trie.o emdb_log.o emdb_config.o emdb_string.o emdb_geo_hash.o emdb_stack.o emdb_event.o emdb_connection.o emdb_serv.o emdb_modules.o emdb_buf.o emdb_resp.o emdb_serv.o 
	ar cr $@ $^

emdb: emdb_server.o
	cc $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)

emdb_cli: emdb_client.o
	cc $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)

.o : .c
	cc -c $^ $(CFLAGS) -o $@
