#include <etdb.h>

static int etdb_set_add_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_set_del_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_set_members_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_set_ismember_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);

static etdb_command_t etdb_list_commands[] = {
  {
    etdb_string("sadd"),
    ETDB_CMD_FLAG_ARG2MORE,
    etdb_set_add_handler,
    etdb_command_padding
  },
  {
    etdb_string("sdel"),
    ETDB_CMD_FLAG_ARG2MORE,
    etdb_set_del_handler,
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

  if(set_name->str.len > 255 )  return -1;

  static uint8_t set_name_str[256];
  memcpy(set_name_str, set_name->str.data, set_name->str.len);
  set_name->str.data = set_name_str; 
  
  do{
    /*** write to db ***/
    if(etdb_database_set_add(&set_name->str, &key->str) < 0)
       return -1;   
    key                  = (etdb_bytes_t*)(key->queue.next);
  }while(key != args);
  return 0;
}

static int 
etdb_set_del_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *set_name = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *key      = (etdb_bytes_t*)(set_name->queue.next);

  if(set_name->str.len > 255 )  return -1;

  static uint8_t set_name_str[256];
  memcpy(set_name_str, set_name->str.data, set_name->str.len);
  set_name->str.data = set_name_str;

  do{
    if(etdb_database_set_del(&set_name->str, &key->str) < 0)
       return -1;
    key                  = (etdb_bytes_t*)(key->queue.next);
  }while(key != args);
  return 0;
}

static int 
etdb_set_members_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *set_name = (etdb_bytes_t*)(args->queue.next->next);
  etdb_database_set_members(&set_name->str, conn->pool_temp, resp);
  return 0;
}

static int 
etdb_set_ismember_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *set_name = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *key      = (etdb_bytes_t*)(set_name->queue.next);

  int contain            = etdb_database_set_ismember(&set_name->str, &key->str); 
  etdb_bytes_t *n        = (etdb_bytes_t*)etdb_palloc(conn->pool_temp, sizeof(etdb_bytes_t));
  if(contain){
    n->str.data = "1";
    n->str.len  = 1;
  }else{
    n->str.data = "0";
    n->str.len  = 1;
  } 
  etdb_queue_insert_tail(&(resp->queue), &(n->queue)); 
  return 0;
}
