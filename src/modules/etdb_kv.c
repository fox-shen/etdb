#include <etdb.h>

static int etdb_kv_set_handler(etdb_bytes_t *args, etdb_connection_t *conn);

static etdb_command_t etdb_kv_commands[] = {
  {
    etdb_string("set"),
    ETDB_CMD_FLAG_WRITE,
    etdb_kv_set_handler,
    etdb_command_padding
  },
  {
    etdb_null_command
  }
};

etdb_module_t etdb_kv_module={
  etdb_string("kv"),
  etdb_kv_commands,
  etdb_module_padding
};

static int etdb_kv_set_handler(etdb_bytes_t *args, etdb_connection_t *conn)
{
  fprintf(stderr, "kv set operation\n"); 
}

