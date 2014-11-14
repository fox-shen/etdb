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

static void
emdb_trie_pop_block(emdb_trie_t *trie, const int64_t bi, int64_t *head_in, int last)
{
  if(last){
    *head_in = 0;
  }else{
    const emdb_trie_block_t *b = trie->block + bi;
    trie->block[b->prev].next  = b->next;
    trie->block[b->next].prev  = b->prev;
    if(bi == *head_in) 
      *head_in = b->next;  
  }
}

static void
emdb_trie_push_block(emdb_trie_t *trie, const int64_t bi, int64_t *head_out, int empty)
{
  emdb_trie_block_t *b  = trie->block + bi;
  if(empty){
    *head_out = b->prev = b->next = bi;
  }else{
    int64_t *tail_out   = &(trie->block[*head_out].prev);
    b->prev             = *tail_out;
    b->next             = *head_out;
    *head_out = *tail_out = trie->block[*tail_out].next = bi;
  }
}

static int64_t
emdb_trie_add_block(emdb_trie_t *trie)
{
  int64_t i;
  if(trie->size == trie->capacity){
    int64_t old_capacity = trie->capacity; 
    trie->capacity     += trie->capacity;

    emdb_trie_node_realloc(&(trie->node),   trie->capacity, old_capacity);
    emdb_trie_ninfo_realloc(&(trie->ninfo), trie->capacity, trie->size);
    emdb_trie_block_realloc(&(trie->block), trie->capacity >> 8, trie->size >> 8);
  }  
  trie->block[trie->size >> 8].ehead = trie->size;
  trie->node[trie->size].base  = -(trie->size + 255);
  trie->node[trie->size].check = -(trie->size + 1);

  for(i = trie->size + 1; i < trie->size + 255; ++i){
    trie->node[i].base  = -(i - 1);
    trie->node[i].check = -(i + 1);
  }
  trie->node[trie->size + 255].base = -(trie->size + 254);
  trie->node[trie->size + 255].check= -(trie->size);

  emdb_trie_push_block(trie, trie->size >> 8, &(trie->block_head_open), !trie->block_head_open);
  trie->size += 256;
  return (trie->size >> 8) - 1; 
}

static void
emdb_trie_transfer_block(emdb_trie_t *trie, const int64_t bi, int64_t* head_in, int64_t* head_out)
{
  emdb_trie_pop_block (trie, bi, head_in, bi == trie->block[bi].next);
  emdb_trie_push_block(trie, bi, head_out, !(*head_out)&&trie->block[bi].num);
}

static int64_t
emdb_trie_find_place(emdb_trie_t *trie)
{
  int64_t e;
  if(trie->block_head_close)  
    e   = trie->block[trie->block_head_close].ehead;
  else if(trie->block_head_open)   
    e   = trie->block[trie->block_head_open].ehead;
  else
    e   = emdb_trie_add_block(trie) << 8;
  assert(trie->node[e].check < 0 && trie->node[e].base < 0);
  return e;
}

static int64_t
emdb_trie_find_place_interval(emdb_trie_t *trie, const uint8_t *first, const uint8_t *last)
{
  int64_t bi = trie->block_head_open;
  if(bi){
    const int64_t bz = trie->block[trie->block_head_open].prev;
    const int16_t nc = (int16_t)(last - first + 1);
    while(1){
      emdb_trie_block_t *b = trie->block + bi;
      if(b->num >= nc && nc <= b->reject){
        int64_t e = b->ehead;
        for(;;){
          const int64_t base  = e ^ *first;
          const uint8_t *p    = first;
          for(; trie->node[base ^ *++p].check < 0; ){
            if(p == last)  return b->ehead = e;
          }
          if((e = -trie->node[e].check) == b->ehead) 
            break;
        }
      }
      b->reject = nc;
      if(b->reject < trie->reject[b->num])
        trie->reject[b->num] = b->reject;
      const int64_t bi_ = b->next;
      if(++b->trial == EMDB_TRIE_MAX_TRIAL)
        emdb_trie_transfer_block(trie, bi, &(trie->block_head_open), &(trie->block_head_close));
      if(bi == bz)  break;
      bi = bi_;
    }
  } 
  return emdb_trie_add_block(trie) << 8; 
}

static int64_t
emdb_trie_pop_empty_node(emdb_trie_t *trie, const int64_t base, const uint8_t label, const int64_t from)
{
  const int64_t e      = base < 0 ? emdb_trie_find_place(trie) : base ^ label;
  const int64_t bi     = e >> 8;
  emdb_trie_node_t  *n = trie->node  + e;
  emdb_trie_block_t *b = trie->block + bi;

  if(--b->num == 0 && bi){
    emdb_trie_transfer_block(trie, bi, &(trie->block_head_close), &(trie->block_head_full));
  }else{
    trie->node[-n->base].check = n->check;
    trie->node[-n->check].base = n->base;
    if(e == b->ehead) 
      b->ehead = -n->check;
    if(bi && b->num == 1 && b->trial != EMDB_TRIE_MAX_TRIAL){ /*** Open to Closed***/
      emdb_trie_transfer_block(trie, bi, &(trie->block_head_open), &(trie->block_head_close));
    }
  }
#ifdef EMDB_TRIE_REDUCED
  n->value = EMDB_TRIE_VALUE_LIMIT;
  n->check = from;
  if(base < 0)
    trie->node[from].base = -(e ^ label) - 1;
#else
  if(label)
    n->base  = -1;
  else
    n->value = 0;
  n->check   = from;
  if(base < 0)
    trie->node[from].base = e ^ label;
#endif
  return e;
}

static void
emdb_trie_push_empty_node(emdb_trie_t *trie, const int64_t e)
{
  const int64_t bi     = e >> 8;
  emdb_trie_block_t *b = trie->block + bi;
  if(++b->num == 1){ /*** full to closed ****/  
    b->ehead            = e;
    trie->node[e].base  = -e;
    trie->node[e].check = -e;
    if(bi)
      emdb_trie_transfer_block(trie, bi, &(trie->block_head_full), &(trie->block_head_close)); /*** full to closed ****/
  }else{
    const int64_t prev       = b->ehead;
    const int64_t next       = -(trie->node[prev].check);
    trie->node[e].base       = -prev;
    trie->node[e].check      = -next;
    trie->node[prev].check   = trie->node[next].base = -e;

    if(b->num == 2 || b->trial == EMDB_TRIE_MAX_TRIAL){ /*** closed to open ***/
      if(bi) emdb_trie_transfer_block(trie, bi, &(trie->block_head_close), &(trie->block_head_open));
    }
    b->trial = 0;
  }
  if(b->reject < trie->reject[b->num])
    b->reject = trie->reject[b->num];
  trie->ninfo[e].sibling = 0;  /*** reset ninfo; no child, no sibling ***/
  trie->ninfo[e].child   = 0;
}

static void
emdb_trie_push_sibling(emdb_trie_t *trie, int64_t from, int64_t base, const uint8_t label, int flag)
{
  uint8_t* c = &trie->ninfo[from].child;
  if(flag && /*label > *c */ !(*c)){
    do{
      c = &trie->ninfo[base ^ *c].sibling;
    }while(/**c && *c < label*/0);
  }
  trie->ninfo[base ^ label].sibling = *c;
  *c = label;
}

static void
emdb_trie_pop_sibling(emdb_trie_t *trie, int64_t from, int64_t base, const uint8_t label)
{
  uint8_t *c = &(trie->ninfo[from].child);
  while(*c != label){
    c = &(trie->ninfo[base ^ *c].sibling);
  }
  *c = trie->ninfo[base ^ label].sibling;
}

static int
emdb_trie_consult(emdb_trie_t *trie, const int64_t base_n, const int64_t base_p, uint8_t c_n, uint8_t c_p)
{
  do{
    c_n = trie->ninfo[base_n ^ c_n].sibling;
    c_p = trie->ninfo[base_p ^ c_p].sibling;
  }while(c_n && c_p);
  return c_p;
}

static uint8_t*
emdb_trie_set_child(emdb_trie_t *trie, uint8_t *p, const int64_t base, uint8_t c, const int label)
{
  --p;
  if(!c){
    *++p = c;
    c = trie->ninfo[base ^ c].sibling;
  }
  /*
  while(c && c < label){
    *++p = c;
    c = trie->ninfo[base ^ c].sibling;
  }*/
  if(label != -1){
    *++p = (uint8_t)label;
  }
  while(c){
    *++p = c;
    c = trie->ninfo[base ^ c].sibling;
  }
  return p;
}

static int64_t   /**** resolve conflict on base_n ^ label_n = base_p ^ label_p ***/
emdb_trie_resolve(emdb_trie_t *trie, int64_t *from_n, const int64_t base_n, const uint8_t label_n)
{
  int64_t to_pn   = base_n ^ label_n;
  int64_t from_p  = trie->node[to_pn].check;
#ifdef EMDB_TRIE_REDUCED
  int64_t base_p  = -(trie->node[from_p].base + 1);
#else
  int64_t base_p  = trie->node[from_p].base;
#endif
  int flag = emdb_trie_consult(trie, base_n, base_p, trie->ninfo[*from_n].child, trie->ninfo[from_p].child);
  uint8_t child[256];
  uint8_t* const first = &child[0];
  uint8_t* const last  = flag ? emdb_trie_set_child(trie, first, base_n, trie->ninfo[*from_n].child, label_n): 
                                emdb_trie_set_child(trie, first, base_p, trie->ninfo[from_p].child,  -1);
  int64_t base = (first == last ? emdb_trie_find_place(trie) : emdb_trie_find_place_interval(trie, first, last)) ^ (*first);
  const int64_t from = flag ? *from_n : from_p;
  const int64_t base_ = flag ? base_n : base_p;
  const uint8_t *p;

  if(flag && *first == label_n)
    trie->ninfo[from].child = label_n;

#ifdef EMDB_TRIE_REDUCED
  trie->node[from].base = -base - 1;
#else
  trie->node[from].base = base;
#endif
  for(p = first; p <= last; ++p){
    const int64_t to  = emdb_trie_pop_empty_node(trie, base, *p, from);
    const int64_t to_ = base_ ^ *p;
    trie->ninfo[to].sibling = (p == last ? 0 : *(p+1));
    if(flag && to_ == to_pn)    continue;
    emdb_trie_node_t *n  = trie->node + to;
    emdb_trie_node_t *n_ = trie->node + to_;

#ifdef EMDB_TRIE_REDUCED
    if((n->base = n_->base) < 0 && *p)
#else
    if((n->base = n_->base) > 0 && *p)
#endif
    {
      uint8_t c  = trie->ninfo[to].child = trie->ninfo[to_].child;
#ifdef EMDB_TRIE_REDUCED
      int64_t nb = -(n->base + 1); 
#else
      int64_t nb = n->base;
#endif
      do{
        trie->node[nb ^ c].check = to;
      }while((c = trie->ninfo[nb ^ c].sibling));
    }

    if(!flag && to_ == *from_n){
      *from_n = to;
    }
    if(!flag && to_ == to_pn){
      emdb_trie_push_sibling(trie, *from_n, to_pn ^ label_n, label_n, 1);
      trie->ninfo[to_].child = 0;
#ifdef EMDB_TRIE_REDUCED
      n_->value = EMDB_TRIE_VALUE_LIMIT;
#else
      if(label_n) 
        n_->base = -1;
      else
        n_->value = 0;
#endif
      n_->check = *from_n;
    }else{
      emdb_trie_push_empty_node(trie, to_);
    }

    if(EMDB_TRIE_NUM_TRACKING_NODES){ /*** keep the traversed node updated**/
      size_t j;
      for(j = 0; trie->tracking_node[j] != 0; ++j){
        if(trie->tracking_node[j] == to_){
          trie->tracking_node[j] = to;
          break;
        }
      }
    }
  }  
  return flag ? base ^ label_n : to_pn;
}

static int64_t
emdb_trie_follow(emdb_trie_t *trie, int64_t *from, const uint8_t label)
{
  int64_t to = 0;
#ifdef EMDB_TRIE_REDUCED
  const int64_t base = -(trie->node[*from].base + 1);
#else
  const int64_t base = trie->node[*from].base;
#endif
  if(base < 0 || trie->node[to = base ^ label].check < 0){
    to = emdb_trie_pop_empty_node(trie, base, label, *from);
    emdb_trie_push_sibling(trie, *from, to ^ label, label, base >= 0);  
  }else if(trie->node[to].check != *from){
    to = emdb_trie_resolve(trie, from, base, label);
  } 
  return to;
}

static void
emdb_trie_update_entry(emdb_trie_t *trie, int64_t *from, uint8_t word, uint8_t sp)
{
#ifdef EMDB_TRIE_REDUCED
  const int64_t val = trie->node[*from].value;
  if(val >= 0 && val != EMDB_TRIE_VALUE_LIMIT){
    const int64_t to = emdb_trie_follow(trie, from, 0);
    trie->node[to].value = val;
  } 
#endif
  /*** encode ***/
  if(!sp){
    if(word == '\0'){
      *from = emdb_trie_follow(trie, from, '%');
      emdb_trie_update_entry(trie, from, '0', 1);
    }else if(word == '%'){
      *from = emdb_trie_follow(trie, from, '%');
      emdb_trie_update_entry(trie, from, '%', 1);
    }else{
      *from  = emdb_trie_follow(trie, from, word);
    }
  }else{
    *from  = emdb_trie_follow(trie, from, word);
  }
}

int
emdb_trie_update(emdb_trie_t *trie, const char *key, size_t len, int64_t value)
{
  int64_t from = 0;
  int64_t pos  = 0;
  if(len == 0)
    return 0;
  
  const uint8_t *key_uint8 = (const uint8_t*)key;
  int sp = 0;
  for( ; pos < len; ++(pos)){
    emdb_trie_update_entry(trie, &from, key_uint8[pos], 0);
  }

#ifdef EMDB_TRIE_REDUCED
  const int64_t to = trie->node[from].value >= 0 ? from : emdb_trie_follow(trie, &from, 0);
  if(trie->node[to].value == EMDB_TRIE_VALUE_LIMIT){
    trie->node[to].value = 0;
  }
#else
  const int64_t to = emdb_trie_follow(trie, &from, 0);
#endif
  trie->node[to].value = value;
  return 1;
}

static int64_t
emdb_trie_find(emdb_trie_t *trie, const char *key, int64_t *from, size_t pos, size_t len)
{
  int64_t to;
  uint8_t *key_int8 = (uint8_t*)key;
  for(; pos < len; ){
    uint8_t word  = key_int8[pos];
    uint8_t again_word = ' ';
    switch(word){
      case '\0':
        word       = '%';
        again_word = '0';
        break;
      case '%':
        word       = '%';
        again_word = '%';
        break;
      default:
        break;
    }
  
AGAIN:
#ifdef EMDB_TRIE_REDUCED
    if(trie->node[*from].value >= 0) break;
    to = -(trie->node[*from].base + 1);
#else
    to = trie->node[*from].base;
#endif
    to         = to ^ word;
    if(trie->node[to].check != *from)
      return EMDB_TRIE_NO_VALUE;
    *from       = to;
    if(again_word != ' '){
      word = again_word;
      again_word = ' ';
      goto AGAIN;
    }
    ++pos;
  }
#ifdef EMDB_TRIE_REDUCED
  if(trie->node[*from].value >= 0)
    return pos == len ? trie->node[*from].value : EMDB_TRIE_NO_PATH; 
  emdb_trie_node_t *n = trie->node + ((-(trie->node[*from].base + 1)) ^ 0);
#else 
  emdb_trie_node_t *n = trie->node + (trie->node[*from].base ^ 0);
#endif
  if(n->check != *from)
    return EMDB_TRIE_NO_VALUE;
  return n->base;
}

int64_t
emdb_trie_exact_match_search(emdb_trie_t *trie, const char *key, size_t len)
{
  int64_t ret  = -1;
  size_t  pos  = 0;
  int64_t from = 0;
  union{ int64_t i; int64_t value;} b;
  b.i = emdb_trie_find(trie, key, &from, pos, len);
  if(b.i == EMDB_TRIE_NO_PATH)
    b.i = EMDB_TRIE_NO_VALUE;
  return b.value;
}

static void
emdb_trie_erase_impl(emdb_trie_t *trie, int64_t from)
{
#ifdef EMDB_TRIE_REDUCED
  int64_t e = trie->node[from].value >= 0 ? from : (-(trie->node[from].base + 1)) ^ 0;
  from = trie->node[e].check;
#else
  int64_t e = trie->node[from].base ^ 0; 
#endif
  uint8_t flag = 0; /*** have sibling ***/
  do{
    emdb_trie_node_t *n = trie->node + from;
#ifdef EMDB_TRIE_REDUCED
    int64_t nb          = -(n->base + 1);
#else
    int64_t nb          = n->base;
#endif
    flag = trie->ninfo[nb ^ trie->ninfo[from].child].sibling;
    if(flag){
      emdb_trie_pop_sibling(trie, from, nb, nb ^ e);
    }
    emdb_trie_push_empty_node(trie, e);
    e    = from;
    from = trie->node[from].check;
  }while(!flag);
}

int
emdb_trie_erase(emdb_trie_t *trie, const char *key, size_t len)
{
  int64_t from = 0, pos = 0;
  int64_t i = emdb_trie_find(trie, key, &from, pos, len);
  if(i == EMDB_TRIE_NO_PATH || i == EMDB_TRIE_NO_VALUE)
    return 0;
  emdb_trie_erase_impl(trie, from);
  return 1;
}

int64_t
emdb_trie_total_size(emdb_trie_t *trie)
{
  return trie->size;
}

int64_t
emdb_trie_nonzero_size(emdb_trie_t *trie)
{
  int64_t i = 0, to = 0;
  for(; to < trie->size; ++to){
    if(trie->node[to].check >= 0) ++i;
  }
  return i;
}

int64_t
emdb_trie_num_keys(emdb_trie_t *trie)
{
  size_t i = 0, to = 0;
  for(; to < trie->size; ++to){
#ifdef EMDB_TRIE_REDUCED
    if(trie->node[to].check >= 0 && trie->node[to].value >= 0)
      ++i;
#else
    if(trie->node[to].check >= 0 && trie->node[trie->node[to].check].base == to)
      ++i;
#endif
  }
  return i;
}
