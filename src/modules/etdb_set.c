#include <etdb.h>

static int etdb_set_add_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_set_rem_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);

#define ETDB_SET_HEAD '3'

static etdb_command_t etdb_list_commands[] = {
  {
    etdb_string("sadd"),
    ETDB_CMD_FLAG_ARG1MORE,
    etdb_set_add_handler,
    etdb_command_padding
  },
  {
    etdb_string("srem"),
    ETDB_CMD_FLAG_ARG1MORE,
    etdb_set_rem_handler,
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
  return 0;
}

static int 
etdb_set_rem_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  return 0;
}
