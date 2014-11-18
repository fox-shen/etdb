#include <etdb.h>

void
TestStringSplit()
{
  etdb_str_t raw_str = etdb_string("I am a good guy");
  etdb_str_t splits[20];
  size_t     split_num = 20;
  size_t ret   = etdb_str_split(&raw_str, ' ', splits, &split_num);
  assert(ret == 5);
  assert(split_num == 5);
  assert(memcmp(splits[0].data, "I",  splits[0].len) == 0);
  assert(memcmp(splits[1].data, "am", splits[1].len) == 0);
  assert(memcmp(splits[2].data, "a",  splits[2].len) == 0);
  assert(memcmp(splits[3].data, "good",  splits[3].len) == 0);
  assert(memcmp(splits[4].data, "guy",  splits[4].len) == 0);

  
}

int
main(int argc, char **argv)
{
  TestStringSplit();
  return 0;
}
