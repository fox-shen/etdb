#ifndef H_ETDB_DATABASE_H
#define H_ETDB_DATABASE_H

typedef struct etdb_value_head_s etdb_value_head_t;
struct etdb_value_head_s{
  size_t    size;
  uint8_t   type;
};

typedef struct etdb_database_s etdb_database_t;
struct etdb_database_s{
  etdb_trie_t  trie;  
};

int etdb_database_init();

/**** db: kv type operation ****/
/***NOTE:
 *   key->str.data[-1] must be valid
 ****/
int etdb_database_kv_set(etdb_str_t *key, const etdb_str_t *value);
int etdb_database_kv_get(etdb_str_t *key, etdb_str_t *value);
int etdb_database_kv_del(etdb_str_t *key);

/**** db: hash type operation ****/
/***NOTE:
 *   key->str.data[-hash_name.str.len - 2] must be vaild.
 *   hash_name->len <= 255 must!
 ****/
int etdb_database_hash_set(etdb_str_t *hash_name, etdb_str_t *key, const etdb_str_t *value);
int etdb_database_hash_get(etdb_str_t *hash_name, etdb_str_t *key, etdb_str_t *value);
int etdb_database_hash_del(etdb_str_t *hash_name, etdb_str_t *key);

/**** db: set operation ***/
/***NOTE:
 *   key->str.data[-set_name->str.len - 2] must be valid
 *   set_name->len <= 255 must
 ****/
int etdb_database_set_add(etdb_str_t *set_name, etdb_str_t *key);
int etdb_database_set_del(etdb_str_t *set_name, etdb_str_t *key);
int etdb_database_set_members(etdb_str_t *set_name, etdb_pool_t *pool, etdb_bytes_t *resp);
int etdb_database_set_ismember(etdb_str_t *set_name, etdb_str_t *key);

/**** db: list operation ***/
int etdb_database_list_lpush(etdb_str_t *list_name, etdb_str_t *value);
int etdb_database_list_rpush(etdb_str_t *list_name, etdb_str_t *value);
int etdb_database_list_lpop(etdb_str_t *list_name, etdb_str_t *value);
int etdb_database_list_rpop(etdb_str_t *list_name, etdb_str_t *value);
int etdb_database_list_ltop(etdb_str_t *list_name, etdb_str_t *value);
int etdb_database_list_rtop(etdb_str_t *list_name, etdb_str_t *value);

/**** db: spatial point index operation ****/
int etdb_database_sp_set(etdb_str_t *list_name, etdb_str_t *value);




/*** sys operation **/
const uint8_t* etdb_database_info_version();
size_t etdb_database_info_mem();
size_t etdb_database_info_keys();





/*** kv , hash, set related opertation ***/
int etdb_database_update(const uint8_t *key, size_t key_len, const uint8_t *value, size_t value_len);
int etdb_database_exact_match(const uint8_t *key, size_t key_len, uint8_t **value, size_t *value_len);
int etdb_database_common_prefix_match(const uint8_t *key, size_t key_len, etdb_pool_t *pool, etdb_bytes_t *resp);
int etdb_database_common_prefix_path_match(const uint8_t *key, size_t key_len, etdb_pool_t *pool, etdb_bytes_t *resp);
int etdb_database_erase(const uint8_t *key, size_t key_len);

#endif
