#include <etdb.h>

etdb_trie_t etdb_command_trie;
extern etdb_module_t* etdb_modules[]; 

int 
etdb_module_init()
{
  int pos = 0, idx = 0;
  etdb_trie_init(&etdb_command_trie); 
  
  for(; etdb_modules[pos] != NULL; pos++){
    if(etdb_modules[pos]->init_handler){
      if( etdb_modules[pos]->init_handler(NULL) < 0 ){  
        return -1;
      }
    }

    if(etdb_modules[pos]->commands == NULL)  continue;

    etdb_id_t value = 0;
    etdb_modules[pos]->module_id = pos;
    value         = pos;
    value         = value << 16;

    idx           = 0;
    while(etdb_modules[pos]->commands[idx].name.data != NULL){
       etdb_id_t value_insert = value | idx;
       if( etdb_trie_update(&etdb_command_trie, 
                            etdb_modules[pos]->commands[idx].name.data,
                            etdb_modules[pos]->commands[idx].name.len,
                            value_insert) < 0 )
         return -1;
       idx++; 
    }
  }
  return 0;
}

etdb_command_t* 
etdb_module_find_command(etdb_str_t *str)
{
  etdb_id_t value_insert = etdb_trie_exact_match_search(&etdb_command_trie, str->data, str->len);
  if(value_insert == ETDB_TRIE_NO_VALUE){
    return NULL;
  }
  etdb_id_t pos = value_insert >> 16;
  etdb_id_t idx = value_insert & 0xffff;
  return &(etdb_modules[pos]->commands[idx]);
}

