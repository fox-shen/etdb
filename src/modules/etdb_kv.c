#include <etdb.h>

static int etdb_kv_init_handler(void *args);
static int etdb_kv_set_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_kv_get_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_kv_del_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_kv_matchall_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_kv_matchlongest_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);

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
    etdb_string("del"),
    ETDB_CMD_FLAG_ARG1,
    etdb_kv_del_handler,
    etdb_command_padding
  },
  {
    etdb_string("matchall"),
    ETDB_CMD_FLAG_ARG1,
    etdb_kv_matchall_handler,
    etdb_command_padding
  },
  {
    etdb_string("matchlongest"),
    ETDB_CMD_FLAG_ARG1,
    etdb_kv_matchlongest_handler,
    etdb_command_padding
  },
  {
    etdb_null_command
  }
};

etdb_module_t etdb_kv_module={
  etdb_string("kv"),
  etdb_kv_commands,
  NULL, 
  etdb_module_padding
};

static int 
etdb_kv_set_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *key   = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *value = (etdb_bytes_t*)(key->queue.next);

  return etdb_database_kv_set(&key->str, &value->str);
}

static int 
etdb_kv_get_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *key    = (etdb_bytes_t*)(args->queue.next->next); 

  etdb_str_t value     = etdb_null_string;
  int ret = etdb_database_kv_get(&key->str, &value); 

  if(ret < 0){
     return -1; 
  }
  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool_temp, sizeof(etdb_bytes_t));
  new_bytes->str.len    =  value.len;
  new_bytes->str.data   =  value.data;
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));
  return 0; 
}

static int 
etdb_kv_del_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *key    = (etdb_bytes_t*)(args->queue.next->next);
  return etdb_database_kv_del(&key->str); 
}

static int 
etdb_kv_matchall_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *key    = (etdb_bytes_t*)(args->queue.next->next);
  size_t match_len;
  etdb_str_t value;

   
  return 0; 
}

static int 
etdb_kv_matchlongest_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *key    = (etdb_bytes_t*)(args->queue.next->next);
  size_t match_len;
  etdb_str_t value; 
  
  if(etdb_database_kv_match_longest(&key->str, &match_len, &value) <  0){
    return -1;
  }
  char len_str[32];
  sprintf(len_str, "%d", match_len);

  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool_temp, 2*sizeof(etdb_bytes_t) + strlen(len_str));
  new_bytes->str.len    =  value.len;
  new_bytes->str.data   =  value.data;
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));

  new_bytes++;
  memcpy(new_bytes + 1, len_str, strlen(len_str));
  new_bytes->str.len    = strlen(len_str);
  new_bytes->str.data   = (uint8_t*)(new_bytes + 1);
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));
  return 0;
}
