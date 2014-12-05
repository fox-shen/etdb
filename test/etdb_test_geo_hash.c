#include <etdb.h>

void
TestGeoHash()
{
  char geo_hash_encode[13] = "\0";
  etdb_geo_hash_encode(30, 120, geo_hash_encode, 12);
  printf("30 120 -> %s\n", geo_hash_encode); 

  //etdb_geo_hash_get_max_cover_iterate(29, 30, -180, 180, 12); 
}

int 
main(int argc, char **argv)
{
  TestGeoHash();
  return 0;
}
