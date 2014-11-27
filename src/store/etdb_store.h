#ifndef H_ETDB_STORE_H
#define H_ETDB_STORE_H

/*** child-sibling structure to store trie infos ***/
typedef struct etdb_index_ninfo_s{
  uint8_t                 sibling; 
  uint8_t                 child;
}etdb_index_ninfo_t;

/*** empty trie nodes in block  ****/
typedef struct etdb_index_block_s{
  etdb_store_id_t         prev;
  etdb_store_id_t         next;
  int16_t                 num;
  int16_t                 reject;   /*** # empty elements: 0 - 256 ***/
  int32_t                 trial;    /*** minimum # branching failed to locate ***/
  etdb_store_id_t         ehead;
}etdb_index_block_t;

/*** trie node ***/
typedef struct etdb_index_node_s{
  union { etdb_store_id_t base; etdb_store_id_t value; };  /** negative means prev empty index **/
  etdb_store_id_t  check;  /** negative means next empty index **/
}etdb_index_node_t;

#define ETDB_INDEX_NINFO_SIZE_PER_PAGE    (ETDB_PAGE_SIZE/sizeof(etdb_index_ninfo_t))
#define ETDB_INDEX_BLOCK_SIZE_PER_PAGE    (ETDB_PAGE_SIZE/sizeof(etdb_index_block_t))
#define ETDB_INDEX_NODE_SIZE_PER_PAGE     (ETDB_PAGE_SIZE/sizeof(etdb_index_node_t))

typedef struct etdb_store_value_s{
  /// TODO: should add.
}etdb_store_value_t;

typedef struct etdb_store_index_node_s{
  char         file[256];
  int          fd;
  etdb_lru_t   lru;
}etdb_store_index_node_t;

typedef struct etdb_store_index_ninfo_s{
  char         file[256];
  int          fd;
  etdb_lru_t   lru;
}etdb_store_index_ninfo_t;

typedef struct etdb_store_index_block_s{
  char         file[256];
  int          fd;
  etdb_lru_t   lru;
}etdb_store_index_block_t;

typedef struct etdb_store_s{
  etdb_id_t   block_head_full;
  etdb_id_t   block_head_close;
  etdb_id_t   block_head_open;
  etdb_id_t   capacity;
  etdb_id_t   size;
  etdb_id_t   reject[257];

  etdb_store_index_node_t  node;
  etdb_store_index_ninfo_t ninfo;
  etdb_store_index_block_t block; 

  char         file[256];
  int          fd;
  etdb_log_t   *log;
}etdb_store_t;

typedef struct etdb_store_option_s{
  uint32_t    index_node_lru_cap;
  uint32_t    index_ninfo_lru_cap;
  uint32_t    index_block_lru_cap;
  char        dir[256];
}etdb_store_option_t;

extern int etdb_init_store(etdb_store_t *store, etdb_log_t *log, etdb_store_option_t *option);

#endif
