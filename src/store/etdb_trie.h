#ifndef H_ETDB_TRIE_H
#define H_ETDB_TRIE_H

#if __WORDSIZE == 64
typedef int64_t etdb_id_t;
#else
typedef int32_t etdb_id_t;
#endif

typedef struct etdb_trie_ninfo_s etdb_trie_ninfo_t;
struct etdb_trie_ninfo_s{
  uint8_t  sibling;
  uint8_t  child;
};

typedef struct etdb_trie_block_s etdb_trie_block_t;
struct etdb_trie_block_s{
  etdb_id_t  prev;
  etdb_id_t  next;
  int16_t    num;      /*** # empty elements: 0 - 256 ***/
  int16_t    reject;   /*** minimum # branching failed to locate ***/
  int32_t    trial;
  etdb_id_t  ehead;   
};

typedef struct etdb_trie_node_s etdb_trie_node_t;
struct etdb_trie_node_s{
  union{ etdb_id_t base; etdb_id_t value; };       /** negative means prev empty index **/
  etdb_id_t check;                                 /** negative means next empty index **/
};

#define ETDB_TRIE_NO_PATH            -1
#define ETDB_TRIE_NO_VALUE           -2
#define ETDB_TRIE_NULL_VALUE         -3

#define ETDB_TRIE_MAX_TRIAL          1

#if __WORDSIZE == 64
#define ETDB_TRIE_VALUE_LIMIT        0xefffffffffffffff
#else
#define ETDB_TRIE_VALUE_LIMIT        0xefffffff
#endif

typedef struct etdb_trie_s etdb_trie_t;
struct etdb_trie_s{
  etdb_trie_node_t    *node;
  etdb_trie_ninfo_t   *ninfo;
  etdb_trie_block_t   *block;

  etdb_id_t            block_head_full;
  etdb_id_t            block_head_close;
  etdb_id_t            block_head_open;
  etdb_id_t            capacity;
  etdb_id_t            size;
  etdb_id_t            reject[257]; 
};

extern etdb_trie_node_t*   NodeAt(etdb_trie_t  *trie, etdb_id_t idx);
extern etdb_trie_ninfo_t*  NinfoAt(etdb_trie_t *trie, etdb_id_t idx);
extern etdb_trie_block_t*  BlockAt(etdb_trie_t *trie, etdb_id_t idx);

/*** initialize trie ***/
extern int
etdb_trie_init(etdb_trie_t *trie);

extern void
etdb_trie_destory(etdb_trie_t *trie);

/*** update trie ****/
extern etdb_id_t
etdb_trie_update(etdb_trie_t *trie, const char *key, size_t len, etdb_id_t value);

/*** search trie ***/
extern etdb_id_t
etdb_trie_exact_match_search(etdb_trie_t *trie, const char *key, size_t len);

extern etdb_id_t
etdb_trie_match_longest_search(etdb_trie_t *trie, const char *key, size_t len, size_t *match_len);

extern void
etdb_trie_common_prefix_search(etdb_trie_t *trie, const char *key, size_t len, etdb_stack_t *stack_result);

extern int
etdb_trie_common_prefix_path_search(etdb_trie_t *trie, const char *key, size_t len, etdb_stack_t *stack_in, etdb_bytes_t *result, etdb_pool_t *pool);

/*** delete trie ***/
extern etdb_id_t
etdb_trie_erase(etdb_trie_t *trie, const char *key, size_t len);

/*** get total size ***/
extern size_t
etdb_trie_total_size(etdb_trie_t *trie);

/*** get nonzero size ****/
extern size_t
etdb_trie_nonzero_size(etdb_trie_t *trie);

/*** get # keys ***/
extern size_t
etdb_trie_num_keys(etdb_trie_t *trie);

#endif
