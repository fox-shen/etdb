#include <etdb.h>

static etdb_str_t etdb_log_title[] = { etdb_string("[DEBUG] "), 
                                       etdb_string("[INFO] "), 
                                       etdb_string("[WARN] "), 
                                       etdb_string("[ERROR] ")
                                     };

int  
etdb_log_init(etdb_log_t *log, const char *log_file, const char *log_level)
{
  log->file_name.data = (uint8_t*)log_file;
  log->file_name.len  = strlen(log_file);

  log->fd = open(log_file, O_RDWR | O_CREAT | O_APPEND , 0644); 
  if(log->fd <  0)  return -1;

  if(strcmp(log_level, "DEBUG") == 0)
    log->log_level = ETDB_LOG_DEBUG;
  else if(strcmp(log_level, "INFO") == 0)
    log->log_level = ETDB_LOG_INFO;
  else if(strcmp(log_level, "WARN") == 0)
    log->log_level = ETDB_LOG_WARN;
  else if(strcmp(log_level, "ERROR") == 0)
    log->log_level = ETDB_LOG_ERR;
  else
    log->log_level = ETDB_LOG_INFO;
  return 0;
}

#define MAX_LOG_BUF_SIZE 1024

void 
etdb_log_print(etdb_log_t *log, int log_level, const char *format, ...)
{
  if(log_level < log->log_level || log_level > ETDB_LOG_ERR)
    return;
 
  static char buf[MAX_LOG_BUF_SIZE]; 
  va_list args;
  va_start(args, format);
  int data_size = vsnprintf(buf, MAX_LOG_BUF_SIZE, format, args);
  va_end(args); 
  write(log->fd, etdb_log_title[log_level].data, etdb_log_title[log_level].len);
  write(log->fd, buf,  data_size);
  write(log->fd, "\n", 1);

  if(log_level >= ETDB_LOG_ERR)   exit(1);  
}

