#include <etdb.h>

static int etdb_kv_init_handler(void *args);
static int etdb_kv_set_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_kv_get_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);

static etdb_command_t etdb_kv_commands[] = {
  {
    etdb_string("set"),
    ETDB_CMD_FLAG_WRITE|ETDB_CMD_FLAG_ARG2,
    etdb_kv_set_handler,
    etdb_command_padding
  },
  {
    etdb_string("get"),
    ETDB_CMD_FLAG_READ|ETDB_CMD_FLAG_ARG1,
    etdb_kv_get_handler,
    etdb_command_padding
  },
  {
    etdb_null_command
  }
};

etdb_module_t etdb_kv_module={
  etdb_string("kv"),
  etdb_kv_commands,
  etdb_kv_init_handler, 
  etdb_module_padding
};

/// test for memory trie
static etdb_trie_t etdb_kv_trie;
static uint64_t    etdb_kv_trie_value_cnt = 0;

static int
etdb_kv_init_handler(void *args)
{
  if(etdb_trie_init(&etdb_kv_trie) < 0)
    return -1; 
  return 0;
}

static int 
etdb_kv_set_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *key   = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *value = (etdb_bytes_t*)(key->queue.next);
 
  int ret = etdb_trie_update(&etdb_kv_trie, key->str.data, key->str.len, ++etdb_kv_trie_value_cnt);   
  return ret;
}

static int 
etdb_kv_get_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *key   = (etdb_bytes_t*)(args->queue.next->next); 
  int64_t value = etdb_trie_exact_match_search(&etdb_kv_trie, key->str.data, key->str.len); 
  if(value < 0){
     return -1; 
  }
  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc_temp(conn->pool, sizeof(etdb_bytes_t));
  char tmp[32];  sprintf(tmp, "V:%d", value);
  new_bytes->str.len    =  strlen(tmp);
  new_bytes->str.data   =  etdb_palloc_temp(conn->pool_temp, new_bytes->str.len);
  memcpy(new_bytes->str.data, tmp, new_bytes->str.len);  

  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));
  return 0; 
}
