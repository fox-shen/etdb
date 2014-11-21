#include <etdb.h>

#define MAX_CONFIG_LINE_SIZE 512

static void
etdb_file_config_parse_line(const char *buf)
{
  int pos = 0, begin = 0, end = strlen(buf);
  
}

int
etdb_init_file_config(char *conf_file)
{
  FILE *fp = fopen(conf_file, "r");
  if(fp == NULL){
    fprintf(stderr, "open config file %s failed!\n", conf_file);
    exit(1);
  }
 
  char buf[MAX_CONFIG_LINE_SIZE];
  while(fgets(buf, MAX_CONFIG_LINE_SIZE, fp) != NULL){
    etdb_file_config_parse_line(buf);
  }
}
