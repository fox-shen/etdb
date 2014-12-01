#include <etdb_store_incs.h>

long int 
etdb_utils_get_timestamp()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*1000000 + tv.tv_usec;
}
