#include <emdb.h>

void
TestUpdate()
{
   long int time_stamp_1 = emdb_utls_get_timestamp();
   emdb_trie_t trie;
   emdb_trie_init(&trie);
   unsigned char buf[100];
   int loop = 900000, idx;
   for(idx = 0; idx < loop; ++idx){
     int i;
     for(i = 0; i < 100; ++i){
       buf[i] = (random())%256;
       if(buf[i] == 0) buf[i] = 'a';
     }
     emdb_trie_update(&trie, buf, 100, idx);
   }
   printf("TestUpdate Time: %d(ms)/%d updates\n", (emdb_utls_get_timestamp() - time_stamp_1)/1000, loop);
   printf("size: %d nonzero-size: %d\n", emdb_trie_total_size(&trie), emdb_trie_nonzero_size(&trie));
}

int 
main(int argc, char** argv)
{
   TestUpdate();
   return 1;
}
