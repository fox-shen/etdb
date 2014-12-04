#include <etdb.h>

static int etdb_hash_init_handler(void *args);
static int etdb_hash_set_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_hash_get_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_hash_del_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);

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

  if(name->str.len > 255)  return -1;
 
  return etdb_database_hash_set(conn->slot, &name->str, &key->str, &value->str);
}

static int 
etdb_hash_get_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *name   = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *key    = (etdb_bytes_t*)(name->queue.next); 

  if(name->str.len > 255)  return -1;

  etdb_str_t    value;
  int ret = etdb_database_hash_get(conn->slot, &name->str, &key->str, &value); 

  if(ret < 0){
     return -1; 
  }
  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool, sizeof(etdb_bytes_t));
  new_bytes->str.len    =  value.len;
  new_bytes->str.data   =  value.data;
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));
  return 0; 
}

static int 
etdb_hash_del_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *name   = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *key    = (etdb_bytes_t*)(name->queue.next);

  if(name->str.len > 255)  return -1;

  return etdb_database_hash_del(conn->slot, &name->str, &key->str); 
}
