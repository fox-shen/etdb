#include "etdb.h"

void
TestErase()
{
   etdb_trie_t trie;
   etdb_trie_init(&trie);

#define N 1000000
#define L 32
   unsigned char** buf_array = (unsigned char**)malloc(sizeof(char*)*N);
   etdb_id_t* value   = (etdb_id_t*)malloc(sizeof(etdb_id_t)*N);
   int i;
   for(i = 0; i < N; ++i){
     buf_array[i] = (char*)malloc(sizeof(char)*L);
     memset(buf_array[i], 0, L);
     int n = rand();
     snprintf(buf_array[i], L, "k%010d", n);
   }

   for(i = 0; i < N; ++i){
     etdb_trie_update(&trie, buf_array[i], strlen(buf_array[i]), 1);
   }
   printf("after update key=%d\n", etdb_trie_num_keys(&trie));
   long int time_stamp_1 = etdb_utls_get_timestamp();
   for(i = 0; i < N; ++i){
     // printf("erase: %d -> %d\n", i, N);
     etdb_id_t v = etdb_trie_erase(&trie, buf_array[i], strlen(buf_array[i]));
     ///assert(v == 0);
   }
   printf("Erase Success, Time: %d(ms)/%d queries\n", (etdb_utls_get_timestamp() - time_stamp_1)/1000, N);
   printf("after erase key=%d\n", etdb_trie_num_keys(&trie));
}

int
main(int argc, char** argv)
{
  TestErase();
  return 0;
}
