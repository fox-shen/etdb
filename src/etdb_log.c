#include <etdb.h>

int  
etdb_log_init(etdb_log_t *log, uint8_t *log_file, int log_level)
{
  log->file_name.data = log_file;
  log->file_name.len  = strlen(log_file);

  log->fd = open(log_file, O_CREAT | O_APPEND | O_RDWR, S_IRUSR); 
  if(log->fd <  0)  return -1;
  log->log_level = log_level;
  return 0;
}

#define MAX_LOG_BUF_SIZE 1024

void 
etdb_log_print(etdb_log_t *log, int log_level, const char *format, ...)
{
  if(log_level < log->log_level)
    return;
 
  static char buf[MAX_LOG_BUF_SIZE]; 
  va_list args;
  va_start(args, format);
  int data_size = vsnprintf(buf, MAX_LOG_BUF_SIZE, format, args);
  va_end(args); 
  write(log->fd, buf,  data_size);
  write(log->fd, "\n", 1);

  if(log_level >= ETDB_LOG_ERR)   exit(1);  
}

