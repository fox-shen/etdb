#include <etdb.h>

void
TestPoolAlloc()
{
  etdb_pool_t *pool = (etdb_pool_t*)etdb_create_pool(8192);

  char *data = etdb_palloc(pool, 20);
  data[19]   = 'a';
  data       = etdb_palloc(pool, 4096);
  data[4095]   = 'w';
  etdb_destroy_pool(pool);
}

int 
main(int argc, char **argv)
{
  TestPoolAlloc();
  return 0;
}
