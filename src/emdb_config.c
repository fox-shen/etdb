#include <emdb.h>

emdb_trie_t emdb_command_trie;
extern emdb_module_t emdb_modules[]; 

int 
emdb_module_init()
{
  int pos = 0, idx = 0;
  emdb_trie_init(&emdb_command_trie); 
  
  while(emdb_modules[pos].module_name.data != NULL){
    int64_t value = 0;
    emdb_modules[pos].module_id = pos;
    value         = pos;
    value         = value << 32;

    idx           = 0;
    while(emdb_modules[pos].commands[idx].name.data != NULL){
       int64_t value_insert = value | idx;
       emdb_trie_update(&emdb_command_trie, 
                        emdb_modules[pos].commands[idx].name.data,
                        emdb_modules[pos].commands[idx].name.len,
                        value_insert);
       idx++; 
    }   
  
    pos++;
  }
  return 0;
}

emdb_command_t* 
emdb_module_find_command(emdb_bytes_t* bytes)
{
  int64_t value_insert = emdb_trie_exact_match_search(&emdb_command_trie, bytes->data, bytes->size);
  if(value_insert == EMDB_TRIE_NO_VALUE){
    fprintf(stderr, "not found command\n");
    return NULL;
  }
  int64_t pos = value_insert >> 32;
  int64_t idx = value_insert & 0xffffffff;
  return &(emdb_modules[pos].commands[idx]);
}
