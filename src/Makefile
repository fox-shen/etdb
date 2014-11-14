#Macros
INCLUDE = -I .
LIBPATH = -lpthread -lm -lrt -lcrypto -lssl -lidn 

CFLAGS = -g -O0 ${INCLUDE}
LIBS = libemdb.a
TESTS = EmdbTestTrie

#Actions
all : clean $(LIBS) $(TESTS)

clean : 
	rm -r -f *.o $(LIB) $(TESTS)

EmdbTestTrie : emdb_test_trie.o
	cc $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)

libemdb.a: emdb_trie.o emdb_utils.o
	ar cr $@ $^	

.o : .c
	cc -c $^ $(CFLAGS) -o $@
