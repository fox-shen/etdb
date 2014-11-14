#ifndef H_EMDB_TRIE_H
#define H_EMDB_TRIE_H

typedef struct emdb_trie_ninfo_s emdb_trie_ninfo_t;
struct emdb_trie_ninfo_s{
  uint8_t  sibling;
  uint8_t  child;
};

typedef struct emdb_trie_block_s emdb_trie_block_t;
struct emdb_trie_block_s{
  int64_t  prev;
  int64_t  next;
  int16_t  num;      /*** # empty elements: 0 - 256 ***/
  int16_t  reject;   /*** minimum # branching failed to locate ***/
  int      trial;
  int64_t  ehead;   
};

typedef struct emdb_trie_node_s emdb_trie_node_t;
struct emdb_trie_node_s{
  union{ int64_t base; int64_t value; };       /** negative means prev empty index **/
  int64_t check;                               /** negative means next empty index **/
};

#define EMDB_TRIE_NUM_TRACKING_NODES 0
#define EMDB_TRIE_VALUE_LIMIT        0xffffffffffff
#define EMDB_TRIE_MAX_TRIAL          1

typedef struct emdb_trie_s emdb_trie_t;
struct emdb_trie_s{
  emdb_trie_node_t    *node; 
  emdb_trie_ninfo_t   *ninfo;
  emdb_trie_block_t   *block;
  int64_t              block_head_full;
  int64_t              block_head_close;
  int64_t              block_head_open;
  int64_t              capacity;
  int64_t              size;
  int64_t              no_delete;
  int64_t              reject[257]; 
  int64_t              tracking_node[EMDB_TRIE_NUM_TRACKING_NODES + 1];
};

/*** initialize trie ***/
extern int
emdb_trie_init(emdb_trie_t *trie);

/*** update trie ****/
extern int
emdb_trie_update(emdb_trie_t *trie, const char *key, size_t len, int64_t value);

/*** delete trie ***/
extern int
erase(emdb_trie_t *trie, const char *key, size_t len);

#endif
