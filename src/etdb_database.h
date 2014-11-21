#ifndef H_ETDB_DATABASE_H
#define H_ETDB_DATABASE_H

typedef struct etdb_database_s etdb_database_t;
struct etdb_database_s{
  etdb_trie_t  trie;  
};

int etdb_database_init();
int etdb_database_update(const uint8_t *key, size_t key_len, const uint8_t *value, size_t value_len);
int etdb_database_exact_match(const uint8_t *key, size_t key_len, uint8_t **value, size_t *value_len);
int etdb_database_erase(const uint8_t *key, size_t key_len);

#endif
