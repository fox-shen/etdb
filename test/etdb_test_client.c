#include <hietdb.h>

etdb_client_t client;

void
TestSet()
{
  etdb_str_t key   = etdb_string("largestone");
  etdb_str_t value = etdb_string("glm");
  etdb_set(&client, &key, &value); 
}

void
TestGet()
{
  etdb_str_t key   = etdb_string("largestone");
  etdb_str_t value;
  etdb_get(&client, &key, &value);
  char buf[1024]   = "\0";
  memcpy(buf, value.data, value.len);
  printf("value = %s\n", buf);
}

int
main(int argc, char **argv)
{
  etdb_client_connect(&client, argv[1], atoi(argv[2]));
  TestSet();
  TestGet();
}
