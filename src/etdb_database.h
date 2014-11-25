#ifndef H_ETDB_DATABASE_H
#define H_ETDB_DATABASE_H

typedef struct etdb_value_head_s etdb_value_head_t;
struct etdb_value_head_s{
  uint16_t  size;
  uint8_t   type;
};

typedef struct etdb_database_s etdb_database_t;
struct etdb_database_s{
  etdb_trie_t  trie;  
};

int etdb_database_init();

/*** kv , hash, set related opertation ***/
int etdb_database_update(const uint8_t *key, size_t key_len, const uint8_t *value, size_t value_len);
int etdb_database_exact_match(const uint8_t *key, size_t key_len, uint8_t **value, size_t *value_len);
int etdb_database_common_prefix_match(const uint8_t *key, size_t key_len, etdb_pool_t *pool, etdb_bytes_t *resp);
int etdb_database_common_prefix_path_match(const uint8_t *key, size_t key_len, etdb_pool_t *pool, etdb_bytes_t *resp);
int etdb_database_erase(const uint8_t *key, size_t key_len);

/*** list operation ***/
int 
etdb_database_list_lpush(const uint8_t *key, size_t key_len, const uint8_t *value, size_t value_len);
int 
etdb_database_list_rpush(const uint8_t *key, size_t key_len, const uint8_t *value, size_t value_len);
int 
etdb_database_list_lpop(const uint8_t *key, size_t key_len, uint8_t **value, size_t *value_len);
int 
etdb_database_list_rpop(const uint8_t *key, size_t key_len, uint8_t **value, size_t *value_len);
int
etdb_database_list_remove(const uint8_t *key, size_t key_len, uint8_t *value, size_t value_len);

/*** sys operation **/
const uint8_t* etdb_database_info_version();
size_t etdb_database_info_mem();
size_t etdb_database_info_keys();

#endif
