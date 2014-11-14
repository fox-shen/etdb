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

void
TestSearchHuge()
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
   printf("after huge search key=%d\n", emdb_trie_num_keys(&trie));
   long int time_stamp_1 = emdb_utls_get_timestamp();
   for(i = 0; i < N; ++i){
     int64_t v = emdb_trie_exact_match_search(&trie, buf_array[i], L);
     assert(v == value[i]);
   }
   printf("Huge Search Success, Time: %d(ms)/%d queries\n", (emdb_utls_get_timestamp() - time_stamp_1)/1000, N);
}

int 
main(int argc, char** argv)
{
  TestSearch();
  TestSearchHuge();
  return 1;
}
