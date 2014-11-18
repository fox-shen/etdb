#Macros
INCLUDE = -I../src
LIBPATH = -lpthread -lm -lrt -lcrypto -lssl -lidn ../src/libetdb.a

CFLAGS = -g -O3 ${INCLUDE}
TESTS = etdb_test_trie_update etdb_test_trie_search etdb_test_trie_erase etdb_test_string etdb_test_alloc

#Actions
all : clean $(TESTS)

clean : 
	rm -r -f *.o $(LIB) $(TESTS)

etdb_test_trie_update : etdb_test_trie_update.o
	cc $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)
etdb_test_trie_search: etdb_test_trie_search.o
	cc $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)
etdb_test_trie_erase: etdb_test_trie_erase.o
	cc $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)
etdb_test_string: etdb_test_string.o
	cc $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)
etdb_test_alloc: etdb_test_alloc.o
	cc $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)

.o : .c
	cc -c $^ $(CFLAGS) -o $@
