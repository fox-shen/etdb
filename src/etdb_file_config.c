#include <etdb.h>

static etdb_trie_t etdb_config_trie;

#define MAX_CONFIG_LINE_SIZE 512

static void
etdb_file_config_parse_line(const char *buf)
{
  int pos = 0, begin = 0, end = strlen(buf) - 1;
  while(pos < end && buf[pos] == ' ')  ++pos;
  if(buf[pos] == '#')  return; 
  begin = pos;
  while(end >= 0 && (buf[end] == ' ' || buf[end] == '\n')) --end;
  if(end <= begin)  return;
 
  etdb_str_t real = { end - begin + 1, (uint8_t*)(buf + begin)};
  etdb_str_t splits[2];
  size_t splits_num = 2;
  etdb_str_split(&real, '=', splits, &splits_num);
  
  if(splits_num == 2){
    etdb_trim(splits);
    etdb_trim(splits + 1);
    size_t *value = etdb_alloc(sizeof(size_t) + splits[1].len + 1);
    *value        = splits[1].len;
    memcpy(value + 1, splits[1].data, splits[1].len);
    *((uint8_t*)value + sizeof(size_t) + splits[1].len) = '\0';

    etdb_trie_update(&etdb_config_trie, splits[0].data, splits[0].len, (intptr_t)(value));  
  } 
}

int
etdb_init_file_config(char *conf_file)
{
  FILE *fp = fopen(conf_file, "r");
  if(fp == NULL){
    fprintf(stderr, "open config file %s failed!\n", conf_file);
    exit(1);
  }
 
  etdb_trie_init(&etdb_config_trie);

  char buf[MAX_CONFIG_LINE_SIZE];
  while(fgets(buf, MAX_CONFIG_LINE_SIZE, fp) != NULL){
    etdb_file_config_parse_line(buf);
  }
}

int 
etdb_file_config_get_int(const char *key, int ini)
{
  etdb_id_t id = etdb_trie_exact_match_search(&etdb_config_trie, key, strlen(key)); 
  if(id < 0)
    return ini;
  int ret;
  size_t *value = (size_t*)id; 
  if( etdb_atoi((uint8_t*)(value + 1), *value, &ret) < 0)
    return ini;
  return ret;
}

const char*
etdb_file_config_get_string(const char *key, const char *ini)
{
  etdb_id_t id = etdb_trie_exact_match_search(&etdb_config_trie, key, strlen(key));
  if(id < 0)
    return ini;
  size_t *value = (size_t*)id;
  if(*value <= 0)  return   ini;
  return (const char*)(value + 1);
}
