#include "emdb.h"

void
TestSearch()
{
   emdb_trie_t trie;
   emdb_trie_init(&trie);
   unsigned char buf[1024];
   int64_t value = 1;
   sprintf(buf, "1234#1234");
   buf[4] = '\0';
   emdb_trie_update(&trie, buf, 8, value);
  
   value = 2;
   sprintf(buf, "you love me too");
   //emdb_trie_update(&trie, buf, strlen(buf), value);

   sprintf(buf, "1234#1234");
   buf[4] = '\0';
   value = emdb_trie_exact_match_search(&trie, buf, 8);
   printf("value = %d key=%d\n", value, emdb_trie_num_keys(&trie));
}

int 
main(int argc, char** argv)
{
  TestSearch();
  return 1;
}
