#ifndef H_ETDB_DATABASE_H
#define H_ETDB_DATABASE_H

typedef struct etdb_value_head_s etdb_value_head_t;
#pragma pack(push, 1)
struct etdb_value_head_s{
  uint32_t  size: 24;
  uint8_t   type;
};
#pragma pop()

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
/***NOTE:
 *   id->str[-1] must be valid
 ***/
#define ETDB_GEO_HASH_PRECISION_LEN                12  
#define ETDB_SPATIAL_POINT_MAX_CNT_IN_SET          100  
typedef struct etdb_value_sp_head_s{
  etdb_value_head_t      common_head;
  double                 lat_d;
  double                 lon_d;
  uint8_t                geohash_code[ETDB_GEO_HASH_PRECISION_LEN];
  uint8_t                indexed_len;
}etdb_value_sp_head_t;

int etdb_database_sp_set(etdb_str_t *id, etdb_str_t *lat, etdb_str_t *lon);
int etdb_database_sp_get(etdb_str_t *id, etdb_str_t *lat, etdb_str_t *lon);
int etdb_database_sp_rect(etdb_str_t *lat1, etdb_str_t *lat2, etdb_str_t *lon1, etdb_str_t *lon2);
int etdb_database_sp_knn(etdb_str_t *id, etdb_str_t *k);

/*** sys operation **/
const uint8_t* etdb_database_info_version();
size_t etdb_database_info_mem();
size_t etdb_database_info_keys();
void etdb_database_sys_bgsave();




/*** kv , hash, set related opertation ***/
int etdb_database_update(const uint8_t *key, size_t key_len, const uint8_t *value, size_t value_len);
int etdb_database_exact_match(const uint8_t *key, size_t key_len, uint8_t **value, size_t *value_len);
int etdb_database_common_prefix_match(const uint8_t *key, size_t key_len, etdb_pool_t *pool, etdb_bytes_t *resp);
int etdb_database_common_prefix_path_match(const uint8_t *key, size_t key_len, etdb_pool_t *pool, etdb_bytes_t *resp);
int etdb_database_erase(const uint8_t *key, size_t key_len);

#endif
