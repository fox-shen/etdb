#include <etdb.h>

static int etdb_set_add_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_set_rem_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_set_members_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_set_ismember_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);

#define ETDB_SET_HEAD '3'

static etdb_command_t etdb_list_commands[] = {
  {
    etdb_string("sadd"),
    ETDB_CMD_FLAG_ARG2MORE,
    etdb_set_add_handler,
    etdb_command_padding
  },
  {
    etdb_string("srem"),
    ETDB_CMD_FLAG_ARG2MORE,
    etdb_set_rem_handler,
    etdb_command_padding
  },
  {
    etdb_string("smembers"),
    ETDB_CMD_FLAG_ARG1,
    etdb_set_members_handler,
    etdb_command_padding
  },
  {
    etdb_string("sismember"),
    ETDB_CMD_FLAG_ARG2,
    etdb_set_ismember_handler,
    etdb_command_padding
  },
  {
    etdb_null_command
  }
};

etdb_module_t etdb_set_module={
  etdb_string("set"),
  etdb_list_commands,
  NULL, 
  etdb_module_padding
};

static int 
etdb_set_add_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *set_name = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *key      = (etdb_bytes_t*)(set_name->queue.next);

  /**** note: name->str.len <= 255 ****/
  uint8_t      *src      = set_name->str.data + set_name->str.len - 1;
  size_t        src_len  = set_name->str.len;
  etdb_str_t   value     = etdb_string("1");
  do{
    uint8_t *dst         = key->str.data  - 1;
    size_t  i            = 0;
    while(i++ < src_len) { *dst-- = *src--; }
    *dst--               = (uint8_t)src_len;
    *dst                 = ETDB_SET_HEAD;

    /*** write to db ***/
    if(etdb_database_update(dst, key->str.len + key->str.data - dst, value.data, value.len) < 0)
       return -1; 
    src                  = dst + 1 + src_len;
    key                  = (etdb_bytes_t*)(key->queue.next);
  }while(key != args);
  return 0;
}

static int 
etdb_set_rem_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *set_name = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *key      = (etdb_bytes_t*)(set_name->queue.next);

  /**** note: name->str.len <= 255 ****/
  uint8_t      *src      = set_name->str.data + set_name->str.len - 1;
  size_t        src_len  = set_name->str.len;
  do{
    uint8_t *dst         = key->str.data  - 1;
    size_t  i            = 0;
    while(i++ < src_len) { *dst-- = *src--; }
    *dst--               = (uint8_t)src_len;
    *dst                 = ETDB_SET_HEAD;

    /*** write to db ***/
    etdb_database_erase(dst, key->str.len + key->str.data - dst);
    src                  = dst + 1 + src_len;
    key                  = (etdb_bytes_t*)(key->queue.next);
  }while(key != args);
  return 0;
}

static int 
etdb_set_members_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *set_name = (etdb_bytes_t*)(args->queue.next->next);
  uint8_t *src           = set_name->str.data - 1;
  *src--                 = (uint8_t)set_name->str.len;
  *src                   = ETDB_SET_HEAD;
  
  etdb_database_common_prefix_path_match(src, set_name->str.len + set_name->str.data - src,
                                         conn->pool_temp, resp);
  return 0;
}

static int 
etdb_set_ismember_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *set_name = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *key      = (etdb_bytes_t*)(set_name->queue.next);
  
  /**** note: name->str.len <= 255 ****/
  uint8_t      *src      = set_name->str.data + set_name->str.len - 1;
  size_t        src_len  = set_name->str.len;
  uint8_t *dst           = key->str.data  - 1;
  size_t  i              = 0;
  while(i++ < src_len) { *dst-- = *src--; }
  *dst--                 = (uint8_t)src_len;
  *dst                   = ETDB_SET_HEAD;

  etdb_bytes_t *n        = (etdb_bytes_t*)etdb_palloc(conn->pool_temp, sizeof(etdb_bytes_t));
  uint8_t *value;
  size_t value_len = 0;
  if(etdb_database_exact_match(dst, key->str.data + key->str.len - dst, &value, &value_len) == 0){
    n->str.data = "1";
    n->str.len  = 1;
  }else{
    n->str.data = "0";
    n->str.len  = 1;
  } 
  etdb_queue_insert_tail(&(resp->queue), &(n->queue)); 
  return 0;
}
