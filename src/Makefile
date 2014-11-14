#Macros
INCLUDE = -I .
LIBPATH = -lpthread -lm -lrt -lcrypto -lssl -lidn 

CFLAGS = -g -O3 ${INCLUDE}
LIBS = libemdb.a

#Actions
all : clean $(LIBS) $(TESTS)

clean : 
	rm -r -f *.o $(LIB)

libemdb.a: emdb_trie.o emdb_time.o emdb_alloc.o emdb_palloc.o emdb_log.o emdb_config.o emdb_string.o emdb_geo_hash.o
	ar cr $@ $^	

.o : .c
	cc -c $^ $(CFLAGS) -o $@
