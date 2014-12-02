#include <etdb.h>

void
TestGeoHash()
{
  char geo_hash_encode[13] = "\0";
  etdb_geo_hash_encode(30, 120, geo_hash_encode, 10);
  printf("30 120 -> %s\n", geo_hash_encode); 

  etdb_geo_hash_get_max_cover_iterate(29, 30, 119, 120, 10); 
}

int 
main(int argc, char **argv)
{
  TestGeoHash();
  return 0;
}
