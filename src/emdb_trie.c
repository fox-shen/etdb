#include "emdb.h"

static int
emdb_trie_node_realloc(emdb_trie_node_t **node, size_t size_n, size_t size_p)
{
  emdb_trie_node_t *p, *r;
  void *tmp = realloc(*node, sizeof(emdb_trie_node_t)*size_n);
  if(!tmp){
     return 0;
  }
  *node = (emdb_trie_node_t*)tmp;
  r = *node + size_n;
  p = *node + size_p;
  for(; p != r; ++p){
    p->base = p->check = 0;
  }
  return 1;   
}

static int
emdb_trie_ninfo_realloc(emdb_trie_ninfo_t **ninfo, size_t size_n, size_t size_p)
{
  emdb_trie_ninfo_t *p, *r;
  void *tmp = realloc(*ninfo, sizeof(emdb_trie_ninfo_t)*size_n);
  if(!tmp){
    return 0;
  } 
  *ninfo = (emdb_trie_ninfo_t*)tmp;
  r = *ninfo + size_n;
  p = *ninfo + size_p;
  for(; p != r; ++p){
    p->sibling = p->child = 0;
  }
  return 1;
}

static int
emdb_trie_block_realloc(emdb_trie_block_t **block, size_t size_n, size_t size_p)
{
  emdb_trie_block_t *p, *r;
  void *tmp = realloc(*block, sizeof(emdb_trie_block_t)*size_n);
  if(!tmp){
    return 0;
  }
  *block = (emdb_trie_block_t*)tmp;
  r = *block + size_n;
  p = *block + size_p;
  for(; p != r; ++p){
    p->prev   = p->next = 0;
    p->num    = 256;
    p->reject = 257;
    p->trial  = 0;
    p->ehead  = 0;
  }
  return 1;
}

int
emdb_trie_init(emdb_trie_t *trie)
{
  int i;
  memset(trie, 0, sizeof(emdb_trie_t));
  if(emdb_trie_node_realloc(&(trie->node),   256, 256) == 0)
    return 0;
  if(emdb_trie_ninfo_realloc(&(trie->ninfo), 256, 0) == 0)
    return 0;
  if(emdb_trie_block_realloc(&(trie->block), 1,   0) == 0)
    return 0;

#ifdef EMDB_TRIE_REDUCED
  trie->node[0].base  = trie->node[0].check = -1; 
#else
  trie->node[0].base  = 0;
  trie->node[0].check = -1; 
#endif
  
  for(i = 1; i < 256; ++i){
    if(i == 1){
      trie->node[i].base  = -255;
      trie->node[i].check = -(i + 1);
    }else if(i == 255){
      trie->node[i].base  = -(i - 1);
      trie->node[i].check = -1;
    }else{
      trie->node[i].base  = -(i - 1);
      trie->node[i].check = -(i + 1);
    }
  }
  trie->block[0].ehead = 1;
  trie->capacity       = trie->size = 256;
  for(i = 0; i < EMDB_TRIE_NUM_TRACKING_NODES; ++i)  trie->tracking_node[i] = 0;
  for(i = 0; i <= 256; ++i) trie->reject[i] = i + 1;
  return 1;
}

static int
emdb_trie_update_impl(emdb_trie_t *trie, const char *key, size_t *from, size_t *pos, size_t len, int64_t value)
{
  if(len == 0 && from == 0)
    return 0;
  const uint8_t *key_int8 = (const uint8_t*)key;
  for( ; *pos < len; ++(*pos)){
#ifdef EMDB_TRIE_REDUCED
    const int64_t val = trie->node[from].value;
    if(val >= 0 && val != EMDB_TRIE_VALUE_LIMIT){
      const int64_t to = emdb_trie_follow(trie, from, 0);
      trie->node[to].
    } 
#else

#endif
  }
}

int
emdb_trie_update(emdb_trie_t *trie, const char *key, size_t len, int64_t value)
{
  size_t from = 0;
  size_t pos  = 0;
  return emdb_trie_update_impl(trie, key, &from, &pos, len, value);
}

int
erase(emdb_trie_t *trie, const char *key, size_t len)
{
  return 1;
}
