#include <stdio.h>
#include <stdlib.h>
#include <hietdb.h>

etdb_client_t client;

void
TestKVSet()
{
  etdb_str_t key   = etdb_string("largestone");
  etdb_str_t value = etdb_string("glm");
  etdb_status_t status = etdb_kv_set(&client, &key, &value); 
  assert(etdb_str_equal(&status, "+OK", sizeof("+OK") - 1) == 0); 
  printf("Test KV Set Success!\n");
}

void
TestKVGet()
{
  etdb_str_t key   = etdb_string("largestone");
  etdb_str_t value;
  etdb_kv_get(&client, &key, &value);
  assert(etdb_str_equal(&value, "glm", sizeof("glm") - 1) == 0);
  printf("Test KV Get Success!\n");
}

void
TestKVDel()
{
  etdb_str_t key   = etdb_string("largestone");
  etdb_status_t status = etdb_kv_del(&client, &key);
  assert(etdb_str_equal(&status, "+OK", sizeof("+OK") - 1) == 0);
  printf("Test KV Del Success!\n");
}

void
TestHashSet()
{
  etdb_str_t tb    = etdb_string("t_hash");
  etdb_str_t key   = etdb_string("largestone");
  etdb_str_t value = etdb_string("glm");

  etdb_status_t status = etdb_hash_set(&client, &tb, &key, &value);
  assert(etdb_str_equal(&status, "+OK", sizeof("+OK") - 1) == 0);
  printf("Test Hash Set Success!\n");
}

void
TestHashGet()
{
  etdb_str_t tb    = etdb_string("t_hash");
  etdb_str_t key   = etdb_string("largestone");
  etdb_str_t value;
  
  etdb_hash_get(&client, &tb, &key, &value);
  assert(etdb_str_equal(&value, "glm", sizeof("glm") - 1) == 0);
  printf("Test Hash Get Success!\n");
}

void
TestHashDel()
{
  etdb_str_t tb    = etdb_string("t_hash");
  etdb_str_t key   = etdb_string("largestone");
  etdb_status_t status = etdb_hash_del(&client, &tb, &key);
  assert(etdb_str_equal(&status, "+OK", sizeof("+OK") - 1) == 0);
  printf("Test Hash Del Success!\n");
}

void 
TestSetSet()
{
  etdb_str_t sn    = etdb_string("t_set");
  etdb_str_t item  = etdb_string("largestone");
  etdb_status_t status = etdb_set_add(&client, &sn, &item);
  assert(etdb_str_equal(&status, "+OK", sizeof("+OK") - 1) == 0);

  etdb_str_t item_glm  = etdb_string("glm");
  status = etdb_set_add(&client, &sn, &item_glm);
  assert(etdb_str_equal(&status, "+OK", sizeof("+OK") - 1) == 0);

  etdb_str_t item_baby  = etdb_string("baby");
  status = etdb_set_add(&client, &sn, &item_baby);
  assert(etdb_str_equal(&status, "+OK", sizeof("+OK") - 1) == 0);

  printf("Test Set Set Success!\n");
}

void
TestSetDel()
{
  etdb_str_t sn    = etdb_string("t_set");
  etdb_str_t item  = etdb_string("largestone");
  etdb_status_t status = etdb_set_del(&client, &sn, &item);
  assert(etdb_str_equal(&status, "+OK", sizeof("+OK") - 1) == 0);
  printf("Test Set Del Success!\n");
}

void 
TestSetMembers()
{
  etdb_str_t sn  = etdb_string("t_set");
  etdb_bytes_t *value;
  etdb_status_t status = etdb_set_members(&client, &sn, &value);
  assert(etdb_str_equal(&status, "+OK", sizeof("+OK") - 1) == 0);  
  printf("Test Set Members Success!\n");
}

void
TestSetIsMember()
{
  etdb_str_t sn  = etdb_string("t_set");
  etdb_str_t item = etdb_string("baby");
  etdb_status_t status = etdb_set_ismember(&client, &sn, &item);
  assert(etdb_str_equal(&status, "+OK", sizeof("+OK") - 1) == 0);
  printf("Test Set IsMember Success!\n");
}

void
TestListLpush()
{
  etdb_str_t ln = etdb_string("t_list");
  etdb_str_t item = etdb_string("papa");
  etdb_status_t status = etdb_lst_lpush(&client, &ln, &item);
  assert(etdb_str_equal(&status, "+OK", sizeof("+OK") - 1) == 0);
  printf("Test List Lpush Success!\n");
}

void
TestListRpush()
{
  etdb_str_t ln = etdb_string("t_list");
  etdb_str_t item = etdb_string("mama");
  etdb_status_t status = etdb_lst_rpush(&client, &ln, &item);
  assert(etdb_str_equal(&status, "+OK", sizeof("+OK") - 1) == 0);
  printf("Test List Rpush Success!\n");
}

void 
TestListLtop()
{
  etdb_str_t ln = etdb_string("t_list");
  etdb_str_t value;
  etdb_status_t status = etdb_lst_ltop(&client, &ln, &value);
  assert(etdb_str_equal(&value, "papa", sizeof("papa") - 1) == 0);
  printf("Test List Ltop Success!\n");
}

void 
TestListRtop()
{
  etdb_str_t ln = etdb_string("t_list");
  etdb_str_t value;
  etdb_status_t status = etdb_lst_rtop(&client, &ln, &value);
  assert(etdb_str_equal(&value, "mama", sizeof("mama") - 1) == 0);
  printf("Test List Rtop Success!\n");
}

void
TestListLpop()
{
  etdb_str_t ln = etdb_string("t_list");
  etdb_status_t status = etdb_lst_lpop(&client, &ln);
  assert(etdb_str_equal(&status, "+OK", sizeof("+OK") - 1) == 0);
  printf("Test List Lpop Success!\n");
}

void 
TestListRpop()
{
  etdb_str_t ln = etdb_string("t_list");
  etdb_status_t status = etdb_lst_rpop(&client, &ln);
  assert(etdb_str_equal(&status, "+OK", sizeof("+OK") - 1) == 0);
  printf("Test List Rpop Success!\n");
}

void 
TestSpSet()
{
  etdb_str_t id = etdb_string("baby");
  etdb_status_t status = etdb_spatial_point_set(&client, &id, 30, 120);
  assert(etdb_str_equal(&status, "+OK", sizeof("+OK") - 1) == 0);
  printf("Test Spatial Point Set Success!\n");
}

void
TestSpGet()
{
  etdb_str_t id = etdb_string("baby");
  double lat, lon;
  int code_len;
  char code[32];
  etdb_status_t status = etdb_spatial_point_get(&client, &id, &lat, &lon, code, &code_len);  
  assert(etdb_str_equal(&status, "+OK", sizeof("+OK") - 1) == 0);
  assert(lat == 30 && lon == 120);
  printf("Test Spatial Point Get Success!\n");
}

void 
TestSpRect()
{
  etdb_bytes_t *value_list = NULL;
  etdb_status_t status = etdb_spatial_point_rect_query(&client, 29, 31, 119, 121, &value_list);
  assert(etdb_str_equal(&status, "+OK", sizeof("+OK") - 1) == 0);
  assert(etdb_queue_count(&(value_list->queue)) ==  2);
  printf("Test Spatial Point Rect Success!\n");
}

int
main(int argc, char **argv)
{
  etdb_client_connect(&client, argv[1], atoi(argv[2]));
  TestKVSet();
  TestKVGet();
  TestKVDel();

  TestHashSet();
  TestHashGet();
  TestHashDel();

  TestSetSet();
  TestSetDel();
  TestSetMembers();
  TestSetIsMember();

  TestListLpush();
  TestListRpush();
  TestListLtop();
  TestListRtop();
  TestListLpop();
  TestListRpop();

  TestSpSet();
  TestSpGet();
  TestSpRect();

  return 0;
}
