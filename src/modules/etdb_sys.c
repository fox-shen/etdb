#include <etdb.h>

static int etdb_sys_info_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_sys_perf_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_sys_use_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_sys_where_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_sys_help_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_sys_bgsave_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_sys_ping_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_sys_sync_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);

static etdb_command_t etdb_sys_commands[] = {
  {
    etdb_string("info"),
    ETDB_CMD_FLAG_NOARG,
    etdb_sys_info_handler,
    etdb_command_padding
  },
  {
    etdb_string("perf"),
    ETDB_CMD_FLAG_ARG1,
    etdb_sys_perf_handler,
    etdb_command_padding
  },
  {
    etdb_string("use"),
    ETDB_CMD_FLAG_ARG1,
    etdb_sys_use_handler,
    etdb_command_padding
  },
  {
    etdb_string("where"),
    ETDB_CMD_FLAG_NOARG,
    etdb_sys_where_handler,
    etdb_command_padding
  },
  {
    etdb_string("help"),
    ETDB_CMD_FLAG_NOARG|ETDB_CMD_FLAG_ARG1,
    etdb_sys_help_handler,
    etdb_command_padding
  },
  {
    etdb_string("bgsave"),
    ETDB_CMD_FLAG_NOARG,
    etdb_sys_bgsave_handler,
    etdb_command_padding
  },
  {
    etdb_string("ping"),
    ETDB_CMD_FLAG_NOARG,
    etdb_sys_ping_handler,
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

  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool_temp, 
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
etdb_sys_perf_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *cmd_str  = (etdb_bytes_t*)(args->queue.next->next);
  etdb_command_t *cmd = etdb_module_find_command(&(cmd_str->str));

  if(cmd == NULL)   return -1;
  
  static char buf[256];
  char temp[32];
  char *pos  = memcpyn(buf, cmd->name.data, cmd->name.len);
  pos        = memcpyn(pos, ":", sizeof(":") - 1);
  pos        = memcpyn(pos, " calls->", sizeof(" calls->") - 1);
  sprintf(temp, "%d", cmd->calls);
  pos        = memcpyn(pos, temp, strlen(temp));
  pos        = memcpyn(pos, " time_proc->", sizeof(" time_proc->") - 1);
  sprintf(temp, "%d", cmd->calls ? cmd->time_proc/cmd->calls : 0);
  pos        = memcpyn(pos, temp, strlen(temp));
  pos        = memcpyn(pos, " time_proc_max->", sizeof(" time_proc_max->") - 1);
  sprintf(temp, "%d", cmd->time_proc_max);
  pos        = memcpyn(pos, temp, strlen(temp)); 

  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool_temp, sizeof(etdb_bytes_t));
  new_bytes->str.data     = buf;
  new_bytes->str.len      = pos - buf;
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));

  return 0;
}

static int 
etdb_sys_use_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *slot_str  = (etdb_bytes_t*)(args->queue.next->next);
  int slot = 0;
  int status = etdb_atoi(slot_str->str.data, slot_str->str.len, &slot);
  if(status < 0 || slot <= 0 || slot > etdb_database_sys_max_slot())  return -1;
  conn->slot = slot - 1;
  return 0;
}

static int 
etdb_sys_where_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  char temp[3];
  sprintf(temp, "%d", conn->slot + 1);
  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool_temp, 
                                       sizeof(etdb_bytes_t) + strlen(temp));
  new_bytes->str.len      =  strlen(temp);
  new_bytes->str.data     =  (uint8_t*)(new_bytes + 1);
  memcpy(new_bytes + 1, temp, strlen(temp));
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));
}

static char etdb_help_msg[] = 
"command list:\n\
set get del                          key-value operation\n\
hset hget hdel                       hashtable operation\n\
sadd sdel smembers sismember         set operation\n\
lpush rpush ltop rtop lpop rpop      list operation\n\
spset spget spdel sprect siknn geo_hash_info      spatial index operation\n\
bgsave                               system command\
";

static int 
etdb_sys_help_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool_temp, sizeof(etdb_bytes_t));
  new_bytes->str.len      =  strlen(etdb_help_msg);
  new_bytes->str.data     =  etdb_help_msg;
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));
  return 0;
}

pid_t etdb_bgsave_pid = 0;

static int 
etdb_sys_bgsave_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  if(etdb_bgsave_pid > 0)  return -1;

  switch(etdb_bgsave_pid = fork())
  {
    case -1:
      return -1;

    case 0:  /*** sub child do save operations ***/
      etdb_database_sys_bgsave(); 
      break;

    default:
      return 0;
  }
  return 0;
}

static int 
etdb_sys_ping_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  return 0;
}

static int 
etdb_sys_sync_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  return -1;
}
