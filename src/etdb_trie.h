#ifndef H_ETDB_TRIE_H
#define H_ETDB_TRIE_H

typedef struct etdb_trie_ninfo_s etdb_trie_ninfo_t;
struct etdb_trie_ninfo_s{
  uint8_t  sibling;
  uint8_t  child;
};

typedef struct etdb_trie_block_s etdb_trie_block_t;
struct etdb_trie_block_s{
  int64_t  prev;
  int64_t  next;
  int16_t  num;      /*** # empty elements: 0 - 256 ***/
  int16_t  reject;   /*** minimum # branching failed to locate ***/
  int      trial;
  int64_t  ehead;   
};

typedef struct etdb_trie_node_s etdb_trie_node_t;
struct etdb_trie_node_s{
  union{ int64_t base; int64_t value; };       /** negative means prev empty index **/
  int64_t check;                               /** negative means next empty index **/
};

//#define ETDB_TRIE_REDUCED

#define ETDB_TRIE_NO_PATH            -2
#define ETDB_TRIE_NO_VALUE           -1
#define ETDB_TRIE_NUM_TRACKING_NODES 0
#define ETDB_TRIE_VALUE_LIMIT        0xefffffffffffffff
#define ETDB_TRIE_MAX_TRIAL          1

typedef struct etdb_trie_s etdb_trie_t;
struct etdb_trie_s{
  etdb_trie_node_t    *node; 
  etdb_trie_ninfo_t   *ninfo;
  etdb_trie_block_t   *block;
  int64_t              block_head_full;
  int64_t              block_head_close;
  int64_t              block_head_open;
  int64_t              capacity;
  int64_t              size;
  int64_t              no_delete;
  int64_t              reject[257]; 
  int64_t              tracking_node[ETDB_TRIE_NUM_TRACKING_NODES + 1];
};

/*** initialize trie ***/
extern int
etdb_trie_init(etdb_trie_t *trie);

extern void
etdb_trie_destory(etdb_trie_t *trie);

/*** update trie ****/
extern int64_t
etdb_trie_update(etdb_trie_t *trie, const char *key, size_t len, int64_t value);

/*** search trie ***/
extern int64_t
etdb_trie_exact_match_search(etdb_trie_t *trie, const char *key, size_t len);

extern void
etdb_trie_common_prefix_search(etdb_trie_t *trie, const char *key, size_t len, int64_t *array, size_t *size, etdb_pool_t *pool);

/*** delete trie ***/
extern int
etdb_trie_erase(etdb_trie_t *trie, const char *key, size_t len);

/*** get total size ***/
extern int64_t
etdb_trie_total_size(etdb_trie_t *trie);

/*** get nonzero size ****/
extern int64_t
etdb_trie_nonzero_size(etdb_trie_t *trie);

/*** get # keys ***/
extern int64_t
etdb_trie_num_keys(etdb_trie_t *trie);

#endif
