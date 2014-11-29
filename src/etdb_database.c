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

/*** db: kv type ****/
#define ETDB_KV_HEAD       '0'
#define etdb_database_encode_kv_head(key) *(key->data - 1) = ETDB_KV_HEAD

int 
etdb_database_kv_set(etdb_str_t *key, const etdb_str_t *value)
{
  etdb_database_encode_kv_head(key);

  etdb_value_head_t *head   = (etdb_value_head_t*)etdb_alloc(sizeof(etdb_value_head_t) + value->len);
  head->size                = value->len;
  head->type                = 0; 
  memcpy(head + 1, value->data, value->len);

  if( etdb_trie_update(&(etdb_database->trie), key->data - 1, key->len + 1, (intptr_t)(head)) < 0){
    etdb_free((void*)head);
    return -1;
  }
  return 0;   
}

int 
etdb_database_kv_get(etdb_str_t *key, etdb_str_t *value)
{
  etdb_database_encode_kv_head(key);

  etdb_id_t p_value  = etdb_trie_exact_match_search(&(etdb_database->trie), key->data - 1, key->len + 1);
  if(p_value < 0)   return -1;

  etdb_value_head_t *head = (etdb_value_head_t*)p_value;
  value->len              = head->size;
  value->data             = (uint8_t*)(head + 1);
  return 0;
}

int 
etdb_database_kv_del(etdb_str_t *key)
{
  etdb_database_encode_kv_head(key);
 
  etdb_id_t p_value  = etdb_trie_erase(&(etdb_database->trie), key->data - 1, key->len + 1);
  if(p_value < 0)  return -1;

  etdb_free((void*)p_value);
  return 0; 
}

/**** db: hash type ****/
#define ETDB_HASH_HEAD     '1'
#define etdb_database_encode_hash_head(hash_name, key)  \
uint8_t *src = hash_name->data + hash_name->len - 1;    \
uint8_t *dst = key->data - 1;                           \
size_t pos = 0;                                         \
while(pos){                                             \
  *dst-- = *src--;                                      \
  --pos;                                                \
}                                                       \
*dst--  =  (uint8_t)hash_name->len;                     \
*dst    =  ETDB_HASH_HEAD

int 
etdb_database_hash_set(etdb_str_t *hash_name, etdb_str_t *key, const etdb_str_t *value)
{
  etdb_database_encode_hash_head(hash_name, key);

  etdb_value_head_t *head   = (etdb_value_head_t*)etdb_alloc(sizeof(etdb_value_head_t) + value->len);
  head->size                = value->len;
  head->type                = 0;
  memcpy(head + 1, value->data, value->len);

  if( etdb_trie_update(&(etdb_database->trie), dst, key->data + key->len - dst, (intptr_t)(head)) < 0){
    etdb_free((void*)head);
    return -1;
  }
  return 0; 
}

int 
etdb_database_hash_get(etdb_str_t *hash_name, etdb_str_t *key, etdb_str_t *value)
{
  etdb_database_encode_hash_head(hash_name, key);

  etdb_id_t p_value = etdb_trie_exact_match_search(&(etdb_database->trie), dst, key->data + key->len - dst);
  if(p_value < 0)   return -1;

  etdb_value_head_t *head = (etdb_value_head_t*)p_value;
  value->len              = head->size;
  value->data             = (uint8_t*)(head + 1);
  return 0; 
}

int 
etdb_database_hash_del(etdb_str_t *hash_name, etdb_str_t *key)
{
  etdb_database_encode_hash_head(hash_name, key);

  etdb_id_t p_value  = etdb_trie_erase(&(etdb_database->trie), dst, key->data + key->len - dst);
  if(p_value < 0)  return -1;

  etdb_free((void*)p_value);
  return 0; 
}

/**** db: set operation ***/
#define ETDB_SET_HEAD     '2'
#define etdb_database_encode_set_head(set_name, key)    \
uint8_t *src = set_name->data + set_name->len - 1;      \
uint8_t *dst = key->data - 1;                           \
size_t pos = 0;                                         \
while(pos){                                             \
  *dst-- = *src--;                                      \
  --pos;                                                \
}                                                       \
*dst--  =  (uint8_t)set_name->len;                      \
*dst    =  ETDB_SET_HEAD

int 
etdb_database_set_add(etdb_str_t *set_name, etdb_str_t *key)
{
  etdb_database_encode_set_head(set_name, key);

  if( etdb_trie_update(&(etdb_database->trie), dst, key->data + key->len - dst, 1) < 0){
    return -1;
  }
  return 0; 
}

int 
etdb_database_set_del(etdb_str_t *set_name, etdb_str_t *key)
{
  etdb_database_encode_set_head(set_name, key);
  etdb_id_t p_value  = etdb_trie_erase(&(etdb_database->trie), dst, key->data + key->len - dst);
  if(p_value < 0)  return -1;
  return 0;
}

int 
etdb_database_set_members(etdb_str_t *set_name, etdb_pool_t *pool, etdb_bytes_t *resp)
{
  static char set_name_key[512];
  etdb_str_t key = {0, set_name_key + 512};

  etdb_database_encode_set_head(set_name, (&key));

  etdb_stack_t stack_in;
  etdb_stack_init(&stack_in, pool, 1, sizeof(uint8_t));
  etdb_trie_common_prefix_path_search(&(etdb_database->trie), dst, key.data + key.len - dst, &stack_in, resp, pool);
  return 0;
}

int 
etdb_database_set_ismember(etdb_str_t *set_name, etdb_str_t *key)
{
  etdb_database_encode_set_head(set_name, key);
  etdb_id_t p_value = etdb_trie_exact_match_search(&(etdb_database->trie), dst, key->data + key->len - dst);
  if(p_value < 0)   return 0;
  return 1;
}

/**** db: list type ****/
#define ETDB_LIST_HEAD     '3'
#define etdb_database_encode_list_head(key) *(key->data - 1) = ETDB_LIST_HEAD

int 
etdb_database_list_lpush(etdb_str_t *list_name, etdb_str_t *value)
{
  etdb_database_encode_list_head(list_name);

  etdb_id_t p_value  = etdb_trie_exact_match_search(&(etdb_database->trie), list_name->data - 1, list_name->len + 1);
  if(p_value < 0){ /**** alloc new list head ****/
     etdb_list_t *head     = etdb_list_new();
     etdb_list_lpush(head, value->data, value->len);
     if( etdb_trie_update(&(etdb_database->trie), list_name->data - 1, list_name->len + 1, (intptr_t)(head)) < 0 ){
       etdb_queue_free(&(head->queue));
       return -1;
     }
  }else{ /*** only lpush ****/
     etdb_list_t *head      = (etdb_list_t*)p_value;
     etdb_list_lpush(head, value->data, value->len);
  }
  return 0;  
}

int 
etdb_database_list_rpush(etdb_str_t *list_name, etdb_str_t *value)
{
  etdb_database_encode_list_head(list_name);

  etdb_id_t p_value  = etdb_trie_exact_match_search(&(etdb_database->trie), list_name->data - 1, list_name->len + 1);
  if(p_value < 0){ /**** alloc new list head ****/
     etdb_list_t *head     = etdb_list_new();
     etdb_list_rpush(head, value->data, value->len);
     if( etdb_trie_update(&(etdb_database->trie), list_name->data - 1, list_name->len + 1, (intptr_t)(head)) < 0){
       etdb_queue_free(&(head->queue));
       return -1;
     }
  }else{ /*** only lpush ****/
     etdb_list_t *head      = (etdb_list_t*)p_value;
     etdb_list_rpush(head, value->data, value->len);
  }
  return 0;   
}

int 
etdb_database_list_lpop(etdb_str_t *list_name, etdb_str_t *value)
{
  etdb_database_encode_list_head(list_name);

  etdb_id_t p_value   = etdb_trie_exact_match_search(&(etdb_database->trie), list_name->data - 1, list_name->len + 1);
  if(p_value < 0){
    return -1;
  }
  etdb_list_t *head = (etdb_list_t*)p_value;
  if(etdb_list_empty(head))  return -1;

  etdb_list_t *l    = (etdb_list_t*)(head->queue.next);
  etdb_queue_remove(&(l->queue));
  value->data       = l->data;
  value->len        = l->size;

  if(etdb_queue_empty(&(head->queue))){
    etdb_id_t p_value  = etdb_trie_erase(&(etdb_database->trie), list_name->data - 1, list_name->len + 1);
    if(p_value < 0)  return -1;
    etdb_free((void*)p_value);
  }
  return 0;
}

int 
etdb_database_list_rpop(etdb_str_t *list_name, etdb_str_t *value)
{
  etdb_database_encode_list_head(list_name);

  etdb_id_t p_value   = etdb_trie_exact_match_search(&(etdb_database->trie), list_name->data - 1, list_name->len + 1);
  if(p_value < 0)   return -1;

  etdb_list_t *head = (etdb_list_t*)p_value;
  if(etdb_list_empty(head))  return -1;

  etdb_list_t *l    = (etdb_list_t*)(head->queue.prev);
  etdb_queue_remove(&(l->queue));
  value->data       = l->data;
  value->len        = l->size;

  if(etdb_queue_empty(&(head->queue))){
    etdb_id_t p_value  = etdb_trie_erase(&(etdb_database->trie), list_name->data - 1, list_name->len + 1);
    if(p_value < 0)  return -1;
    etdb_free((void*)p_value);
  }
  return 0;
}

int 
etdb_database_list_ltop(etdb_str_t *list_name, etdb_str_t *value)
{
  etdb_database_encode_list_head(list_name);
  etdb_id_t p_value   = etdb_trie_exact_match_search(&(etdb_database->trie), list_name->data - 1, list_name->len + 1);
  if(p_value < 0){
    return -1;
  }
  etdb_list_t *head = (etdb_list_t*)p_value;
  if(etdb_list_empty(head))  return -1;

  etdb_list_t *l    = (etdb_list_t*)(head->queue.next);
  value->data       = l->data;
  value->len        = l->size;
  return 0;
}

int 
etdb_database_list_rtop(etdb_str_t *list_name, etdb_str_t *value)
{
  etdb_database_encode_list_head(list_name);
  etdb_id_t p_value   = etdb_trie_exact_match_search(&(etdb_database->trie), list_name->data - 1, list_name->len + 1);
  if(p_value < 0)   return -1;

  etdb_list_t *head = (etdb_list_t*)p_value;
  if(etdb_list_empty(head))  return -1;

  etdb_list_t *l    = (etdb_list_t*)(head->queue.prev);
  value->data       = l->data;
  value->len        = l->size;
  return 0;
}

/**** db: sptial point type ***/
#define ETDB_SPATIAL_POINT_HEAD1   'a'     /// used for id->lon lat geohash.
#define ETDB_SPATIAL_POINT_HEAD2   'b'     /// used for geohash predix -> lon lat geohash.

int 
etdb_database_sp_set(etdb_str_t *id, etdb_str_t *lat, etdb_str_t *lon)
{
  int ret = 0;
  double lat_d, lon_d;
  if( etdb_atof(lat->data, lat->len, &lat_d) == -1 ||
      etdb_atof(lon->data, lon->len, &lon_d) == -1 ||
      lat_d < -90 || lat_d > 90 || lon_d < -180 || lon_d > 180)
  {
    return -1;
  }
  *(id->data - 1)    = ETDB_SPATIAL_POINT_HEAD1;
  etdb_id_t p_value  = etdb_trie_exact_match_search(&(etdb_database->trie), id->data - 1, id->len + 1);
  if(p_value < 0) /*** first set location ***/
  {

  }else /*** later append location ***/
  {
    etdb_value_head_t *head = (etdb_value_head_t*)p_value;
           
  } 
}

int 
etdb_database_sp_get(etdb_str_t *id, etdb_str_t *lat, etdb_str_t *lon)
{

}

int 
etdb_database_sp_rect(etdb_str_t *lat1, etdb_str_t *lat2, etdb_str_t *lon1, etdb_str_t *lon2)
{

}

int 
etdb_database_sp_knn(etdb_str_t *id, etdb_str_t *k)
{
 
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

void 
etdb_database_sys_bgsave()
{
  const char *db_file_name = etdb_file_config_get_string("DB_FILE_NAME", "etdb.rdb");
  FILE *fp = fopen(db_file_name, "wb");
  if(fp == NULL){

  }else{
    /*** write trie head ***/
    fwrite(&(etdb_database->trie.block_head_full),  sizeof(etdb_id_t), 1, fp);
    fwrite(&(etdb_database->trie.block_head_close), sizeof(etdb_id_t), 1, fp);
    fwrite(&(etdb_database->trie.block_head_open),  sizeof(etdb_id_t), 1, fp);
    fwrite(&(etdb_database->trie.capacity), sizeof(etdb_id_t), 1, fp);
    fwrite(&(etdb_database->trie.size), sizeof(etdb_id_t), 1, fp);
    fwrite(&(etdb_database->trie.reject), sizeof(etdb_id_t), 257, fp);
  
    /*** write node ***/
    fwrite(&(etdb_database->trie.node), sizeof(etdb_trie_node_t), etdb_database->trie.capacity, fp);
    /*** write ninfo ***/
    fwrite(&(etdb_database->trie.ninfo), sizeof(etdb_trie_ninfo_t), etdb_database->trie.capacity, fp);
    /*** write block ***/
    fwrite(&(etdb_database->trie.block), sizeof(etdb_trie_block_t), etdb_database->trie.capacity >> 8, fp);

    fclose(fp);
  }
  exit(0);  
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

