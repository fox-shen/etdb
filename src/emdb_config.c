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
