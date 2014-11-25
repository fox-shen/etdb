#include <etdb.h>

static int etdb_list_lpush_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_list_rpush_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_list_lpop_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_list_rpop_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);

#define ETDB_LIST_HEAD '2'

static etdb_command_t etdb_list_commands[] = {
  {
    etdb_string("lpush"),
    ETDB_CMD_FLAG_ARG2MORE,
    etdb_list_lpush_handler,
    etdb_command_padding
  },
  {
    etdb_string("rpush"),
    ETDB_CMD_FLAG_ARG2MORE,
    etdb_list_rpush_handler,
    etdb_command_padding
  },
  {
    etdb_string("lpop"),
    ETDB_CMD_FLAG_ARG1,
    etdb_list_lpop_handler,
    etdb_command_padding
  },
  {
    etdb_string("rpop"),
    ETDB_CMD_FLAG_ARG1,
    etdb_list_rpop_handler,
    etdb_command_padding
  },
  {
    etdb_null_command
  }
};

etdb_module_t etdb_list_module={
  etdb_string("list"),
  etdb_list_commands,
  NULL, 
  etdb_module_padding
};

static int 
etdb_list_lpush_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *name  = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *value = (etdb_bytes_t*)(name->queue.next);
 
  /**** note: name->str.len <= 255 ****/
  uint8_t *dst        = name->str.data  - 1;
  *dst--              = (uint8_t)name->str.len;
  *dst                = ETDB_LIST_HEAD;

  return etdb_database_list_lpush(dst, name->str.len + name->str.data - dst, 
                                  value->str.data, value->str.len);
}

static int 
etdb_list_rpush_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *name   = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *value  = (etdb_bytes_t*)(name->queue.next); 

  /**** note: name->str.len <= 255 ****/
  uint8_t *dst         = name->str.data  - 1;
  *dst--               = (uint8_t)name->str.len;
  *dst                 = ETDB_LIST_HEAD;

  return etdb_database_list_rpush(dst, name->str.len + name->str.data - dst,
                                  value->str.data, value->str.len);
}

static int 
etdb_list_lpop_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *name   = (etdb_bytes_t*)(args->queue.next->next);

  /**** note: name->str.len <= 255 ****/
  uint8_t *dst         = name->str.data  - 1;
  *dst--               = (uint8_t)name->str.len;
  *dst                 = ETDB_LIST_HEAD;

  uint8_t *value       = NULL;
  size_t value_len     = 0;
  int ret = etdb_database_list_lpop(dst, name->str.len + name->str.data - dst,
                                    &value, &value_len);
  if(ret < 0){
     return -1;
  }
  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool, sizeof(etdb_bytes_t) + value_len);
  new_bytes->str.data     = (uint8_t*)new_bytes + sizeof(etdb_bytes_t);
  new_bytes->str.len      =  value_len;
  memcpy(new_bytes->str.data, value, value_len);
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));

  etdb_free(value - sizeof(etdb_queue_t) - sizeof(size_t));
  return 0;
}

static int
etdb_list_rpop_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *name   = (etdb_bytes_t*)(args->queue.next->next);

  /**** note: name->str.len <= 255 ****/
  uint8_t *dst         = name->str.data  - 1;
  *dst--               = (uint8_t)name->str.len;
  *dst                 = ETDB_LIST_HEAD;

  uint8_t *value       = NULL;
  size_t value_len     = 0;
  int ret = etdb_database_list_rpop(dst, name->str.len + name->str.data - dst,
                                    &value, &value_len);
  if(ret < 0){
     return -1;
  }
  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool, sizeof(etdb_bytes_t) + value_len);
  new_bytes->str.data     = (uint8_t*)new_bytes + sizeof(etdb_bytes_t);
  new_bytes->str.len      = value_len;
  memcpy(new_bytes->str.data, value, value_len);
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));

  etdb_free(value - sizeof(etdb_queue_t) - sizeof(size_t));
  return 0;
}
