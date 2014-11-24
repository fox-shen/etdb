#include <etdb.h>

static int etdb_zset_add_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_zset_rem_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);

#define ETDB_SET_HEAD '3'

static etdb_command_t etdb_list_commands[] = {
  {
    etdb_string("zadd"),
    ETDB_CMD_FLAG_ARG1MORE,
    etdb_zset_add_handler,
    etdb_command_padding
  },
  {
    etdb_string("zrem"),
    ETDB_CMD_FLAG_ARG1MORE,
    etdb_zset_rem_handler,
    etdb_command_padding
  },
  {
    etdb_null_command
  }
};

etdb_module_t etdb_zset_module={
  etdb_string("zset"),
  etdb_list_commands,
  NULL, 
  etdb_module_padding
};

static int 
etdb_zset_add_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  return 0;
}

static int 
etdb_zset_rem_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  return 0;
}
