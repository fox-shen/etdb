#ifndef H_ETDB_DATABASE_H
#define H_ETDB_DATABASE_H

#define TYPE_KV                  1
#define TYPE_HASH                2
#define TYPE_SET                 3
#define TYPE_LIST                4
#define TYPE_SPATIAL_POINT       5

typedef struct etdb_value_head_s etdb_value_head_t;
#pragma pack(push, 1)
struct etdb_value_head_s{
  uint8_t   type;
  uint32_t  size: 24;
};
#pragma pop()

typedef struct etdb_database_s etdb_database_t;
#define ETDB_DATABASE_MAX_SLOT 16
struct etdb_database_s{
  etdb_trie_t  trie[ETDB_DATABASE_MAX_SLOT];  
};

int etdb_database_init();

/**** db: kv type operation ****/
/***NOTE:
 *   key->str.data[-1] must be valid
 ****/
int etdb_database_kv_set(int sl, etdb_str_t *key, const etdb_str_t *value);
int etdb_database_kv_get(int sl, etdb_str_t *key, etdb_str_t *value);
int etdb_database_kv_del(int sl, etdb_str_t *key);
int etdb_database_kv_match_longest(int sl, etdb_str_t *key, size_t *match_len, etdb_str_t *value);

/**** db: hash type operation ****/
/***NOTE:
 *   key->str.data[-hash_name.str.len - 2] must be vaild.
 *   hash_name->len <= 255 must!
 ****/
int etdb_database_hash_set(int sl, etdb_str_t *hash_name, etdb_str_t *key, const etdb_str_t *value);
int etdb_database_hash_get(int sl, etdb_str_t *hash_name, etdb_str_t *key, etdb_str_t *value);
int etdb_database_hash_del(int sl, etdb_str_t *hash_name, etdb_str_t *key);

/**** db: set operation ***/
/***NOTE:
 *   key->str.data[-set_name->str.len - 2] must be valid
 *   set_name->len <= 255 must
 ****/
int etdb_database_set_add(int sl, etdb_str_t *set_name, etdb_str_t *key);
int etdb_database_set_del(int sl, etdb_str_t *set_name, etdb_str_t *key);
int etdb_database_set_members(int sl, etdb_str_t *set_name, etdb_pool_t *pool, etdb_bytes_t *resp);
int etdb_database_set_ismember(int sl, etdb_str_t *set_name, etdb_str_t *key);

/**** db: list operation ***/
int etdb_database_list_lpush(int sl, etdb_str_t *list_name, etdb_str_t *value);
int etdb_database_list_rpush(int sl, etdb_str_t *list_name, etdb_str_t *value);
int etdb_database_list_lpop(int sl, etdb_str_t *list_name);
int etdb_database_list_rpop(int sl, etdb_str_t *list_name);
int etdb_database_list_ltop(int sl, etdb_str_t *list_name, etdb_str_t *value);
int etdb_database_list_rtop(int sl, etdb_str_t *list_name, etdb_str_t *value);

/**** db: spatial point index operation ****/
/***NOTE:
 *   id->str[-1] must be valid
 ***/
#define ETDB_GEO_HASH_PRECISION_LEN                9 
#pragma pack(push, 1) 
typedef struct etdb_value_sp_head_s{
  uint8_t                type;
  double                 lat_d;
  double                 lon_d;
  uint8_t                geo_hash_code[ETDB_GEO_HASH_PRECISION_LEN];
  uint8_t                indexed_len:6;
  uint8_t                ref:2;
}etdb_value_sp_head_t;
#pragma pack(pop)

int etdb_database_sp_set(int sl, etdb_str_t *id, etdb_str_t *lat, etdb_str_t *lon, etdb_pool_t *pool);
int etdb_database_sp_get(int sl, etdb_str_t *id, double *lat, double *lon, char *geo_hash_code);
int etdb_database_sp_del(int sl, etdb_str_t *id, etdb_pool_t *pool);
int etdb_database_sp_rect(int sl, etdb_str_t *lat1, etdb_str_t *lat2, 
                          etdb_str_t *lon1, etdb_str_t *lon2,
                          etdb_pool_t *pool, etdb_bytes_t *resp);
int etdb_database_sp_knn(int sl, etdb_str_t *id, etdb_str_t *k);

/*** sys operation **/
const uint8_t* etdb_database_info_version();
size_t etdb_database_info_mem();
size_t etdb_database_info_keys();
void etdb_database_sys_bgsave();
void etdb_database_sys_load();
int etdb_database_sys_max_slot();

#endif
