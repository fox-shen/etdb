#ifndef H_ETDB_LOG_H
#define H_ETDB_LOG_H

typedef struct etdb_log_s etdb_log_t;
struct etdb_log_s{
  etdb_str_t  file_name;
  int         fd;
   
};

#endif
