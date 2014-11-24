#ifndef H_ETDB_LOG_H
#define H_ETDB_LOG_H

typedef struct etdb_log_s etdb_log_t;
struct etdb_log_s{
  etdb_str_t  file_name;
  int         fd;       
  int         log_level;
};

#define ETDB_LOG_DEBUG    1
#define ETDB_LOG_INFO     2
#define ETDB_LOG_WARN     3
#define ETDB_LOG_ERR      4

int  etdb_log_init(etdb_log_t *log,  uint8_t *log_file, int log_level);
void etdb_log_print(etdb_log_t *log, int log_level, const char *format, ...);

#endif
