#include "emdb.h"

void
TestErase()
{
   emdb_trie_t trie;
   emdb_trie_init(&trie);
#define N 1000000
#define L 20
   unsigned char** buf_array = (unsigned char**)malloc(sizeof(char*)*N);
   int64_t* value   = (int64_t*)malloc(sizeof(int64_t)*N);
   int i;
   for(i = 0; i < N; ++i){
     buf_array[i] = (char*)malloc(sizeof(char)*L);
     memset(buf_array[i], 0, L);
     value[i]     = 0;
   }

   for(i = 1; i < N; ++i){
     int j = 0;
     memcpy(buf_array[i], buf_array[i-1], L);
     do{
       buf_array[i][j]++;
       if(buf_array[i][j] == 255){
         buf_array[i][j] = 0;
         ++j;
       }else
         break;
     }while(j < L);
     value[i] = i;
   }

   for(i = 0; i < N; ++i){
     emdb_trie_update(&trie, buf_array[i], L, value[i]);
   }
   printf("after update key=%d\n", emdb_trie_num_keys(&trie));
   long int time_stamp_1 = emdb_utls_get_timestamp();
   for(i = 0; i < N; ++i){
     // printf("erase: %d -> %d\n", i, N);
     int64_t v = emdb_trie_erase(&trie, buf_array[i], L);
     assert(v != 0);
   }
   printf("Erase Success, Time: %d(ms)/%d queries\n", (emdb_utls_get_timestamp() - time_stamp_1)/1000, N);
   printf("after erase key=%d\n", emdb_trie_num_keys(&trie));
}

int
main(int argc, char** argv)
{
  TestErase();
  return 0;
}
