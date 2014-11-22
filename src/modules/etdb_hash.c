#include <etdb.h>

static int etdb_hash_init_handler(void *args);
static int etdb_hash_set_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_hash_get_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_hash_del_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);

#define ETDB_HASH_HEAD '1'

static etdb_command_t etdb_hash_commands[] = {
  {
    etdb_string("hset"),
    ETDB_CMD_FLAG_ARG3,
    etdb_hash_set_handler,
    etdb_command_padding
  },
  {
    etdb_string("hget"),
    ETDB_CMD_FLAG_ARG2,
    etdb_hash_get_handler,
    etdb_command_padding
  },
  {
    etdb_string("hdel"),
    ETDB_CMD_FLAG_ARG2,
    etdb_hash_del_handler,
    etdb_command_padding
  },
  {
    etdb_null_command
  }
};

etdb_module_t etdb_hash_module={
  etdb_string("hash"),
  etdb_hash_commands,
  NULL, 
  etdb_module_padding
};

static int 
etdb_hash_set_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *name  = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *key   = (etdb_bytes_t*)(name->queue.next);
  etdb_bytes_t *value = (etdb_bytes_t*)(key->queue.next);
 
  /**** note: name->str.len <= 255 ****/
  uint8_t *src        = name->str.data + name->str.len -1;
  uint8_t *dst        = key->str.data  - 1;
  while(*src != '\n') { *dst-- = *src--; } 
  *dst--              = (uint8_t)name->str.len;
  *dst                = ETDB_HASH_HEAD;

  return etdb_database_update(dst, key->str.len + key->str.data - dst + 1, 
                              value->str.data, value->str.len);
}

static int 
etdb_hash_get_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *name   = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *key    = (etdb_bytes_t*)(name->queue.next); 

  /**** note: name->str.len <= 255 ****/
  uint8_t *src        = name->str.data + name->str.len -1;
  uint8_t *dst        = key->str.data  - 1;
  while(*src != '\n') { *dst-- = *src--; }
  *dst--              = (uint8_t)name->str.len;
  *dst                = ETDB_HASH_HEAD;

  uint8_t *value = NULL;
  size_t value_len = 0;
  int ret = etdb_database_exact_match(dst, key->str.len + key->str.data - dst + 1, 
                                      &value, &value_len); 

  if(ret < 0){
     return -1; 
  }
  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc_temp(conn->pool, sizeof(etdb_bytes_t));
  new_bytes->str.len    =  value_len;
  new_bytes->str.data   =  value;
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));
  return 0; 
}

static int 
etdb_hash_del_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *name   = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *key    = (etdb_bytes_t*)(name->queue.next);

  /**** note: name->str.len <= 255 ****/
  uint8_t *src        = name->str.data + name->str.len -1;
  uint8_t *dst        = key->str.data  - 1;
  while(*src != '\n') { *dst-- = *src--; }
  *dst--              = (uint8_t)name->str.len;
  *dst                = ETDB_HASH_HEAD;

  return etdb_database_erase(dst, key->str.len + key->str.data - dst + 1); 
}
