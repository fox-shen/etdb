#ifndef H_ETDB_LRU_H
#define H_ETDB_LRU_H

typedef struct etdb_lru_node_s{
  etdb_queue_t        hash_link;
  etdb_queue_t        lru_link;
  etdb_store_id_t     page_id;
  uint8_t             *page_value;
}etdb_lru_node_t; 

typedef struct etdb_lru_s{
  etdb_lru_node_t     *hash_head;
  etdb_lru_node_t     lru_head; 
  int                 fd;
  uint32_t            capacity;
  uint32_t            size; 
  uint32_t            hash_head_size;
  etdb_pool_t         *pool;
  etdb_log_t          *log;
}etdb_lru_t;

int etdb_init_lru(etdb_lru_t  *lru, etdb_log_t *log, int fd, uint32_t capacity);
int etdb_destory_lru(etdb_lru_t *lru);

uint8_t* etdb_lru_fetch(etdb_lru_t *lru, etdb_store_id_t page_id);
uint8_t* etdb_lru_alloc_entry(etdb_lru_t *lru, etdb_store_id_t page_id);

#endif
