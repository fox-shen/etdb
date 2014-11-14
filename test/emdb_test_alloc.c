#include <emdb.h>

void
TestPoolAlloc()
{
  emdb_pool_t *pool = (emdb_pool_t*)emdb_create_pool(8192);

  char *data = emdb_palloc(pool, 20);
  data[19]   = 'a';
  data       = emdb_palloc(pool, 4096);
  data[4095]   = 'w';
  emdb_destroy_pool(pool);
}

int 
main(int argc, char **argv)
{
  TestPoolAlloc();
  return 0;
}