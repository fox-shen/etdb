#Macros
INCLUDE = -I../src
LIBPATH = -lpthread -lm -lrt -lcrypto -lssl -lidn ../src/libemdb.a

CFLAGS = -g -O3 ${INCLUDE}
TESTS = emdb_test_trie_update emdb_test_trie_search emdb_test_trie_erase

#Actions
all : clean $(TESTS)

clean : 
	rm -r -f *.o $(LIB) $(TESTS)

emdb_test_trie_update : emdb_test_trie_update.o
	cc $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)

emdb_test_trie_search: emdb_test_trie_search.o
	cc $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)

emdb_test_trie_erase: emdb_test_trie_erase.o
	cc $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)

.o : .c
	cc -c $^ $(CFLAGS) -o $@
