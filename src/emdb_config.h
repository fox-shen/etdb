#ifndef H_EMDB_CONFIG_H
#define H_EMDB_CONFIG_H

typedef int (*emdb_command_handler)(void *serv, emdb_connection_t *conn); 

#define EMDB_CMD_FLAG_READ      (1<<0)
#define EMDB_CMD_FLAG_WRITE     (1<<1)
#define EMDB_CMD_FLAG_BACKEND   (1<<2)
#define EMDB_CMD_FLAG_THREAD    (1<<3)

typedef struct emdb_command_s emdb_command_t;
struct emdb_command_s{
  emdb_str_t               name;
  uint8_t                  flags;
  emdb_command_handler     handler;
  uint64_t                 calls;
  long int                 time_wait;
  long int                 time_proc;
};

#define emdb_command_padding 0, 0, 0
#define emdb_null_command {emdb_null_string, 0, NULL, 0, 0, 0}

typedef struct emdb_module_s emdb_module_t;
struct emdb_module_s{
  emdb_str_t               module_name;
  emdb_command_t           *commands;  
  int                      module_id;
};

#define emdb_null_module {emdb_null_string, NULL}

int emdb_module_init();

#endif
