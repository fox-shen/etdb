#include <etdb.h>

etdb_database_t *etdb_database = NULL;

int 
etdb_database_init()
{
  if(etdb_database == NULL){
    etdb_database = (etdb_database_t*)etdb_alloc(sizeof(etdb_database_t));
    if( etdb_trie_init(&(etdb_database->trie)) < 0 )
      return -1;
  }
  return 0;
}

int 
etdb_database_update(const uint8_t *key, size_t key_len, const uint8_t *value, size_t value_len)
{
#ifndef DISK_VERSION
  uint32_t *nvalue = etdb_alloc(sizeof(uint32_t) + value_len);
  *nvalue          = value_len;
  memcpy(nvalue + 1, value, value_len);
  int64_t p_value  = etdb_trie_update(&(etdb_database->trie), key, key_len, (intptr_t)(nvalue)); 
  if(p_value != 0){
    etdb_free((void*)p_value);
  }
#else
  
#endif
  return 0;
}

int 
etdb_database_exact_match(const uint8_t *key, size_t key_len, uint8_t **value, size_t *value_len)
{
#ifndef DISK_VERSION
  int64_t p_value  = etdb_trie_exact_match_search(&(etdb_database->trie), key, key_len);
  if(p_value < 0)   return -1;
  
  uint32_t *nvalue = (uint32_t*)p_value;
  *value_len       = *nvalue;
  *value           = (uint8_t*)(nvalue + 1);
#else
  
#endif
  return 0;
}

int 
etdb_database_erase(const uint8_t *key, size_t key_len)
{
#ifndef DISK_VERSION
  int64_t p_value  = etdb_trie_erase(&(etdb_database->trie), key, key_len); 
  if(p_value < 0)  return -1;
  
  etdb_free((uint32_t*)p_value);
  return 0;
#else
  return 0;
#endif
}
