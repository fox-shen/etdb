#ifndef H_ETDB_CONFIG_H
#define H_ETDB_CONFIG_H

typedef int (*etdb_command_handler)(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp); 
typedef int (*etdb_init_handler)(void *args);

#define ETDB_CMD_FLAG_READ      (1<<0)
#define ETDB_CMD_FLAG_WRITE     (1<<1)
#define ETDB_CMD_FLAG_BACKEND   (1<<2)
#define ETDB_CMD_FLAG_THREAD    (1<<3)

#define ETDB_CMD_FLAG_NOARG     (1<<4)
#define ETDB_CMD_FLAG_ARG1      (1<<5)
#define ETDB_CMD_FLAG_ARG2      (1<<6)
#define ETDB_CMD_FLAG_ARG3      (1<<7)
#define ETDB_CMD_FLAG_ARG4      (1<<8)

typedef struct etdb_command_s etdb_command_t;
struct etdb_command_s{
  etdb_str_t               name;
  uint32_t                 flags;
  etdb_command_handler     handler;
  uint64_t                 calls;
  long int                 time_wait;
  long int                 time_proc;
};

#define etdb_command_padding 0, 0, 0
#define etdb_null_command etdb_null_string, 0, NULL, 0, 0, 0

#define etdb_module_padding 0

typedef struct etdb_module_s etdb_module_t;
struct etdb_module_s{
  etdb_str_t               module_name;
  etdb_command_t           *commands;  
  etdb_init_handler        init_handler;
  int                      module_id;
};

#define etdb_null_module {etdb_null_string, NULL}

int etdb_module_init();
etdb_command_t* etdb_module_find_command(etdb_str_t *str);

#endif
