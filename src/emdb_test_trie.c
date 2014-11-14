#include <emdb.h>

void
TestUpdate()
{
   long int time_stamp_1 = emdb_utls_get_timestamp();
   emdb_trie_t trie;
   emdb_trie_init(&trie);
   unsigned char buf[100];
   int loop = 300, idx;
   for(idx = 0; idx < loop; ++idx){
     int i;
     for(i = 0; i < 100; ++i){
       buf[i] = (random())%256;
     }
     emdb_trie_update(&trie, buf, 10, idx);
   }
   printf("TestUpdate Time: %d(ms)/%d updates\n", (emdb_utls_get_timestamp() - time_stamp_1)/1000, loop);
}

int 
main(int argc, char** argv)
{
   TestUpdate();
   return 1;
}
