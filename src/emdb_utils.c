#include <emdb.h>

long int 
emdb_utls_get_timestamp()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*1000000 + tv.tv_usec;
}
