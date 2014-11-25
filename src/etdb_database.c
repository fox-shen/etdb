#include <etdb.h>
#include <etdb_list.h>

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
  uint32_t *nvalue = etdb_alloc(sizeof(uint32_t) + value_len);
  *nvalue          = value_len;
  memcpy(nvalue + 1, value, value_len);

  if( etdb_trie_update(&(etdb_database->trie), key, key_len, (intptr_t)(nvalue)) < 0){ 
    etdb_free((void*)nvalue);
    return -1;
  }
  return 0;
}

int 
etdb_database_exact_match(const uint8_t *key, size_t key_len, uint8_t **value, size_t *value_len)
{
  etdb_id_t p_value  = etdb_trie_exact_match_search(&(etdb_database->trie), key, key_len);
  if(p_value < 0)   return -1;
  
  uint32_t *nvalue = (uint32_t*)p_value;
  *value_len       = *nvalue;
  *value           = (uint8_t*)(nvalue + 1);
  return 0;
}

int 
etdb_database_common_prefix_match(const uint8_t *key, size_t key_len, etdb_pool_t *pool, etdb_bytes_t *resp)
{
  etdb_stack_t stack_result;
  etdb_stack_init(&stack_result, pool, 1, sizeof(etdb_id_t));
  etdb_trie_common_prefix_search(&(etdb_database->trie), key, key_len, &stack_result); 

  while(!etdb_stack_empty(&stack_result)){
    etdb_id_t old           = *(etdb_id_t*)etdb_stack_pop(&stack_result);

    etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc(pool, sizeof(etdb_bytes_t));
    new_bytes->str.data     = (uint8_t*)old + sizeof(uint32_t);
    new_bytes->str.len      = *((uint32_t*)old);  

    etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue)); 
  }

  return 0;
}

int 
etdb_database_common_prefix_path_match(const uint8_t *key, size_t key_len, etdb_pool_t *pool, etdb_bytes_t *resp)
{
  etdb_stack_t stack_in;
  etdb_stack_init(&stack_in, pool, 1, sizeof(uint8_t));
  etdb_trie_common_prefix_path_search(&(etdb_database->trie), key, key_len, &stack_in, resp, pool); 
}

int 
etdb_database_erase(const uint8_t *key, size_t key_len)
{
  etdb_id_t p_value  = etdb_trie_erase(&(etdb_database->trie), key, key_len); 
  if(p_value < 0)  return -1;
  
  etdb_free((void*)p_value);
  return 0;
}

int 
etdb_database_list_lpush(const uint8_t *key, size_t key_len, const uint8_t *value, size_t value_len)
{
  etdb_id_t p_value  = etdb_trie_exact_match_search(&(etdb_database->trie), key, key_len);
  if(p_value < 0){ /**** alloc new list head ****/
     etdb_list_t *head     = etdb_list_new();
     etdb_list_lpush(head, value, value_len);
     if( etdb_trie_update(&(etdb_database->trie), key, key_len, (intptr_t)(head)) < 0){
       etdb_queue_free(&(head->queue));
       return -1;
     }
  }else{ /*** only lpush ****/
     etdb_list_t *head      = (etdb_list_t*)p_value;
     etdb_list_lpush(head, value, value_len);
  }
  return 0;
}

int 
etdb_database_list_rpush(const uint8_t *key, size_t key_len, const uint8_t *value, size_t value_len)
{
  etdb_id_t p_value  = etdb_trie_exact_match_search(&(etdb_database->trie), key, key_len);
  if(p_value < 0){ /**** alloc new list head ****/
     etdb_list_t *head     = etdb_list_new();
     etdb_list_rpush(head, value, value_len);
     if( etdb_trie_update(&(etdb_database->trie), key, key_len, (intptr_t)(head)) < 0){
       etdb_queue_free(&(head->queue));
       return -1;
     }
  }else{ /*** only lpush ****/
     etdb_list_t *head      = (etdb_list_t*)p_value;
     etdb_list_rpush(head, value, value_len);
  }
  return 0;
}

int 
etdb_database_list_lpop(const uint8_t *key, size_t key_len, uint8_t **value, size_t *value_len)
{
  etdb_id_t p_value   = etdb_trie_exact_match_search(&(etdb_database->trie), key, key_len);
  if(p_value < 0)   return -1;

  etdb_list_t *head = (etdb_list_t*)p_value;
  if(etdb_list_empty(head))  return -1;

  etdb_list_t *l    = (etdb_list_t*)(head->queue.next);
  etdb_queue_remove(&(l->queue));
  *value            = l->data;
  *value_len        = l->size;

  if(etdb_queue_empty(&(head->queue))){
    etdb_database_erase(key, key_len);
  }
  return 0;
}

int 
etdb_database_list_rpop(const uint8_t *key, size_t key_len, uint8_t **value, size_t *value_len)
{
  etdb_id_t p_value   = etdb_trie_exact_match_search(&(etdb_database->trie), key, key_len);
  if(p_value < 0)   return -1;

  etdb_list_t *head = (etdb_list_t*)p_value;
  if(etdb_list_empty(head))  return -1;
  
  etdb_list_t *l    = (etdb_list_t*)(head->queue.prev);
  etdb_queue_remove(&(l->queue));
  *value            = l->data;
  *value_len        = l->size;

  if(etdb_queue_empty(&(head->queue))){
    etdb_database_erase(key, key_len);
  }
  return 0;
}

int
etdb_database_list_remove(const uint8_t *key, size_t key_len, uint8_t *value, size_t value_len)
{
  etdb_id_t p_value    = etdb_trie_exact_match_search(&(etdb_database->trie), key, key_len);
  if(p_value < 0)   return -1;

  etdb_list_t *head  = (etdb_list_t*)p_value;
  etdb_list_t *l     = (etdb_list_t*)(head->queue.next);
  for(; l != head; l = (etdb_list_t*)(l->queue.next)) {
    if(l->size != value_len )  continue;
    if(memcmp(l->data, value, l->size) == 0){
      etdb_queue_remove(&(l->queue));
      if(etdb_queue_empty(&(head->queue))){
        etdb_database_erase(key, key_len);
      }
      return 0;
    }
  }
  return -1;
}

const uint8_t* 
etdb_database_info_version()
{
  return ETDB_VERSION;
}

size_t 
etdb_database_info_mem()
{
  return etdb_trie_total_size(&(etdb_database->trie))*sizeof(etdb_trie_node_t); 
}

size_t 
etdb_database_info_keys()
{
  return etdb_trie_num_keys(&(etdb_database->trie)); 
}
