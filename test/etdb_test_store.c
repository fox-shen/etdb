#include <etdb.h>

void
TestUpdate()
{
  etdb_store_t store;
  etdb_store_option_t option;

  option.index_node_lru_cap   = 2000;
  option.index_ninfo_lru_cap  = 2000;
  option.index_block_lru_cap  = 100;
  sprintf(option.dir, ".");

  etdb_log_t log;
  etdb_log_init(&log, "log.txt", "DEBUG");

  etdb_init_store(&store, &log, &option); 
}

int
main(int argc, char **argv)
{
  TestUpdate();
}
