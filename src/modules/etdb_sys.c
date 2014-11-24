#include <etdb.h>

static int etdb_sys_info_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_sys_bgsave_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_sys_sync_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);

static etdb_command_t etdb_sys_commands[] = {
  {
    etdb_string("info"),
    ETDB_CMD_FLAG_NOARG,
    etdb_sys_info_handler,
    etdb_command_padding
  },
  {
    etdb_string("help"),
    ETDB_CMD_FLAG_NOARG|ETDB_CMD_FLAG_ARG1,
    etdb_sys_info_handler,
    etdb_command_padding
  },
  {
    etdb_string("bgsave"),
    ETDB_CMD_FLAG_NOARG,
    etdb_sys_bgsave_handler,
    etdb_command_padding
  },
  {
    etdb_string("sync"),
    ETDB_CMD_FLAG_NOARG,
    etdb_sys_sync_handler,
    etdb_command_padding
  },
  {
    etdb_null_command
  }
};

etdb_module_t etdb_sys_module={
  etdb_string("sys"),
  etdb_sys_commands,
  NULL, 
  etdb_module_padding
};

static int 
etdb_sys_info_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  const uint8_t *info_version  = etdb_database_info_version();
  char           info_mem_str[32], info_keys_str[32];
  size_t         info_mem      = etdb_database_info_mem();
  sprintf(info_mem_str,  "%d", info_mem);
  size_t         info_keys     = etdb_database_info_keys(); 
  sprintf(info_keys_str, "%d", info_keys);

  size_t         resp_len      = sizeof("version: ") - 1 + strlen(info_version) +
                                 sizeof("\nmem: ") - 1  + strlen(info_mem_str) +
                                 sizeof("\nkeys: ") - 1 + strlen(info_keys_str);

  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc_temp(conn->pool, 
                                                            sizeof(etdb_bytes_t) + resp_len);
  uint8_t *pos = (uint8_t*)new_bytes + sizeof(etdb_bytes_t);
  pos          = memcpyn(pos, "version: ", sizeof("version: ") - 1);
  pos          = memcpyn(pos, info_version, strlen(info_version));
  pos          = memcpyn(pos, "\nmem: ",  sizeof("\nmem: ") - 1);
  pos          = memcpyn(pos, info_mem_str, strlen(info_mem_str));
  pos          = memcpyn(pos, "\nkeys: ", sizeof("\nkeys: ") - 1);
  pos          = memcpyn(pos, info_keys_str, strlen(info_keys_str));

  new_bytes->str.len      =  resp_len;
  new_bytes->str.data     =  (uint8_t*)new_bytes + sizeof(etdb_bytes_t);
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));
  return 0;
}

static int 
etdb_sys_bgsave_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  return -1;
}

static int 
etdb_sys_sync_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  return -1;
}
