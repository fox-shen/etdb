#include <etdb.h>

void
TestLru()
{
  etdb_store_t store;
  etdb_store_option_t option;

  option.index_node_lru_cap   = 50000;
  option.index_ninfo_lru_cap  = 2000;
  option.index_block_lru_cap  = 100;
  sprintf(option.dir, ".");

  etdb_log_t log;
  etdb_log_init(&log, "log.txt", "DEBUG");

  etdb_init_store(&store, &log, &option); 

  etdb_lru_t *node_lru = &(store.node.lru);
  int i;
  int total = 0;
  int loop = 0;

  long int time_stamp_1 = etdb_utils_get_timestamp();
  for(loop = 0; loop < 20; ++loop){
    for(i = 50000; i > 0; --i){
      uint8_t* ptr = etdb_lru_fetch(node_lru, i);  
      total += *ptr;
    }
  }
  printf("ok %d time: %d\n", total, (etdb_utils_get_timestamp() - time_stamp_1)/1000);
}

int
main(int argc, char **argv)
{
  TestLru();
}
