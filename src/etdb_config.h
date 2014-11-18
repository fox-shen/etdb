#ifndef H_ETDB_CONFIG_H
#define H_ETDB_CONFIG_H

typedef int (*etdb_command_handler)(void *serv, etdb_connection_t *conn); 

#define ETDB_CMD_FLAG_READ      (1<<0)
#define ETDB_CMD_FLAG_WRITE     (1<<1)
#define ETDB_CMD_FLAG_BACKEND   (1<<2)
#define ETDB_CMD_FLAG_THREAD    (1<<3)

typedef struct etdb_command_s etdb_command_t;
struct etdb_command_s{
  etdb_str_t               name;
  uint8_t                  flags;
  etdb_command_handler     handler;
  uint64_t                 calls;
  long int                 time_wait;
  long int                 time_proc;
};

#define etdb_command_padding 0, 0, 0
#define etdb_null_command {etdb_null_string, 0, NULL, 0, 0, 0}

typedef struct etdb_module_s etdb_module_t;
struct etdb_module_s{
  etdb_str_t               module_name;
  etdb_command_t           *commands;  
  int                      module_id;
};

#define etdb_null_module {etdb_null_string, NULL}

int etdb_module_init();
etdb_command_t* etdb_module_find_command(etdb_bytes_t* bytes);

#endif
