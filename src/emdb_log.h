#ifndef H_EMDB_LOG_H
#define H_EMDB_LOG_H

typedef struct emdb_log_s emdb_log_t;
struct emdb_log_s{
  emdb_str_t  file_name;
  int         fd;
   
};

#endif
