#include <etdb.h>

static int etdb_list_lpush_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_list_rpush_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_list_lpop_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_list_rpop_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_list_ltop_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_list_rtop_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);

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
    etdb_string("ltop"),
    ETDB_CMD_FLAG_ARG1,
    etdb_list_ltop_handler,
    etdb_command_padding
  },
  {
    etdb_string("rtop"),
    ETDB_CMD_FLAG_ARG1,
    etdb_list_rtop_handler,
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

  return etdb_database_list_lpush(conn->slot, &name->str, &value->str);
}

static int 
etdb_list_rpush_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *name   = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *value  = (etdb_bytes_t*)(name->queue.next);

  return etdb_database_list_rpush(conn->slot, &name->str, &value->str);
}

static int 
etdb_list_lpop_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *name   = (etdb_bytes_t*)(args->queue.next->next);
  etdb_str_t value;
  int ret = etdb_database_list_lpop(conn->slot, &name->str, &value);
  if(ret < 0){
     return -1;
  }
  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool, sizeof(etdb_bytes_t) + value.len);
  new_bytes->str.data     = (uint8_t*)new_bytes + sizeof(etdb_bytes_t);
  new_bytes->str.len      =  value.len;
  memcpy(new_bytes->str.data, value.data, value.len);
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));

  etdb_free(value.data - sizeof(etdb_queue_t) - sizeof(size_t));
  return 0;
}

static int
etdb_list_rpop_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *name   = (etdb_bytes_t*)(args->queue.next->next);
  etdb_str_t value;
  int ret = etdb_database_list_rpop(conn->slot, &name->str, &value);
  if(ret < 0){
     return -1;
  }
  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool, sizeof(etdb_bytes_t) + value.len);
  new_bytes->str.data     = (uint8_t*)new_bytes + sizeof(etdb_bytes_t);
  new_bytes->str.len      = value.len;
  memcpy(new_bytes->str.data, value.data, value.len);
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));

  etdb_free(value.data - sizeof(etdb_queue_t) - sizeof(size_t));
  return 0;
}

int 
etdb_list_ltop_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *name   = (etdb_bytes_t*)(args->queue.next->next);
  etdb_str_t value;
  int ret = etdb_database_list_ltop(conn->slot, &name->str, &value);
  if(ret < 0){
     return -1;
  }
  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool, sizeof(etdb_bytes_t) + value.len);
  new_bytes->str.data     = (uint8_t*)new_bytes + sizeof(etdb_bytes_t);
  new_bytes->str.len      =  value.len;
  memcpy(new_bytes->str.data, value.data, value.len);
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));
  return 0;
}

int 
etdb_list_rtop_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *name   = (etdb_bytes_t*)(args->queue.next->next);
  etdb_str_t value;
  int ret = etdb_database_list_rtop(conn->slot, &name->str, &value);
  if(ret < 0){
     return -1;
  }
  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool, sizeof(etdb_bytes_t) + value.len);
  new_bytes->str.data     = (uint8_t*)new_bytes + sizeof(etdb_bytes_t);
  new_bytes->str.len      = value.len;
  memcpy(new_bytes->str.data, value.data, value.len);
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));
  return 0;
}
