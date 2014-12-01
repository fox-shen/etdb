#include <etdb.h>

void
TestUpdate(const char* text_file)
{
   FILE* fp = fopen(text_file, "r");
   if(fp == NULL){
     fprintf(stderr, "open file %s failed\n", text_file);
     return;
   }
   long int time_stamp_1 = etdb_utils_get_timestamp();
   etdb_trie_t trie;
   etdb_trie_init(&trie);
   unsigned char buf[1024];

   etdb_id_t value = 0;
   while(fgets(buf, 1024, fp)){
     etdb_trie_update(&trie, buf, strlen(buf) - 1, value++);
   }

   printf("TestUpdate Time: %d(ms)/%d updates\n", (etdb_utils_get_timestamp() - time_stamp_1)/1000, value);
   printf("size: %d nonzero-size: %d key num: %d\n", etdb_trie_total_size(&trie), etdb_trie_nonzero_size(&trie), etdb_trie_num_keys(&trie));
}

void
TestRandomUpdate()
{
  long int time_stamp_1 = etdb_utils_get_timestamp();
  etdb_trie_t trie;
  etdb_trie_init(&trie);
  unsigned char buf[100];
  int loop = 1000000, cnt = 0;
  for(; cnt < loop; ++cnt){
     int i = 0;
     for(; i < 100; i++){
       buf[i] = random()%256;
       //if(buf[i] == 0) buf[i] = 'a';
     }
     etdb_trie_update(&trie, buf, 20, cnt);
  }
  printf("TestUpdate Time: %d(ms)/%d updates\n", (etdb_utils_get_timestamp() - time_stamp_1)/1000, loop);
  printf("size: %d nonzero-size: %d key num: %d\n", etdb_trie_total_size(&trie), etdb_trie_nonzero_size(&trie), etdb_trie_num_keys(&trie));
}

int 
main(int argc, char** argv)
{
   if(argc >= 2)
     TestUpdate(argv[1]);
   else
     TestRandomUpdate();
   return 1;
}
