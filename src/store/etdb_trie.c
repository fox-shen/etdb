#include "etdb.h"

static int
etdb_trie_node_realloc(etdb_trie_node_t **node, size_t size_n, size_t size_p)
{
  etdb_trie_node_t *p, *r;
  void *tmp = etdb_realloc(*node, sizeof(etdb_trie_node_t)*size_n);
  if(!tmp){
     return 0;
  }
  *node = (etdb_trie_node_t*)tmp;
  r = *node + size_n;
  p = *node + size_p;
  for(; p != r; ++p){
    p->base = p->check = 0;
  }
  return 1;   
}

static int
etdb_trie_ninfo_realloc(etdb_trie_ninfo_t **ninfo, size_t size_n, size_t size_p)
{
  etdb_trie_ninfo_t *p, *r;
  void *tmp = etdb_realloc(*ninfo, sizeof(etdb_trie_ninfo_t)*size_n);
  if(!tmp){
    return 0;
  } 
  *ninfo = (etdb_trie_ninfo_t*)tmp;
  r = *ninfo + size_n;
  p = *ninfo + size_p;
  for(; p != r; ++p){
    p->sibling = p->child = 0;
  }
  return 1;
}

static int
etdb_trie_block_realloc(etdb_trie_block_t **block, size_t size_n, size_t size_p)
{
  etdb_trie_block_t *p, *r;
  void *tmp = etdb_realloc(*block, sizeof(etdb_trie_block_t)*size_n);
  if(!tmp){
    return 0;
  }
  *block = (etdb_trie_block_t*)tmp;
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
etdb_trie_init(etdb_trie_t *trie)
{
  int i;
  memset(trie, 0, sizeof(etdb_trie_t));
  if(etdb_trie_node_realloc(&(trie->node),   256, 256) == 0)
    return -1;
  if(etdb_trie_ninfo_realloc(&(trie->ninfo), 256, 0) == 0)
    return -1;
  if(etdb_trie_block_realloc(&(trie->block), 1,   0) == 0)
    return -1;

  trie->node[0].base  = 0;
  trie->node[0].check = -1; 
  
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
  for(i = 0; i < ETDB_TRIE_NUM_TRACKING_NODES; ++i)  trie->tracking_node[i] = 0;
  for(i = 0; i <= 256; ++i) trie->reject[i] = i + 1;
  return 0;
}

void
etdb_trie_destory(etdb_trie_t *trie)
{
  if(trie->node != NULL){
    etdb_free(trie->node);
    trie->node = NULL;
  }
  if(trie->ninfo != NULL){
    etdb_free(trie->ninfo);
    trie->ninfo = NULL;
  }
  if(trie->block != NULL){
    etdb_free(trie->block);
    trie->block = NULL;
  }
}

static void
etdb_trie_pop_block(etdb_trie_t *trie, const etdb_id_t bi, etdb_id_t *head_in, int last)
{
  if(last){
    *head_in = 0;
  }else{
    const etdb_trie_block_t *b = trie->block + bi;
    trie->block[b->prev].next  = b->next;
    trie->block[b->next].prev  = b->prev;
    if(bi == *head_in) 
      *head_in = b->next;  
  }
}

static void
etdb_trie_push_block(etdb_trie_t *trie, const etdb_id_t bi, etdb_id_t *head_out, int empty)
{
  etdb_trie_block_t *b  = trie->block + bi;
  if(empty){
    *head_out = b->prev = b->next = bi;
  }else{
    etdb_id_t *tail_out   = &(trie->block[*head_out].prev);
    b->prev             = *tail_out;
    b->next             = *head_out;
    *head_out = *tail_out = trie->block[*tail_out].next = bi;
  }
}

static etdb_id_t
etdb_trie_add_block(etdb_trie_t *trie)
{
  etdb_id_t i;
  if(trie->size == trie->capacity){
    etdb_id_t old_capacity  = trie->capacity; 
    trie->capacity         += trie->capacity;

    etdb_trie_node_realloc(&(trie->node),   trie->capacity, old_capacity);
    etdb_trie_ninfo_realloc(&(trie->ninfo), trie->capacity, trie->size);
    etdb_trie_block_realloc(&(trie->block), trie->capacity >> 8, trie->size >> 8);
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

  etdb_trie_push_block(trie, trie->size >> 8, &(trie->block_head_open), !trie->block_head_open);
  trie->size += 256;
  return (trie->size >> 8) - 1; 
}

static void
etdb_trie_transfer_block(etdb_trie_t *trie, const etdb_id_t bi, etdb_id_t *head_in, etdb_id_t *head_out)
{
  etdb_trie_pop_block (trie, bi, head_in, bi == trie->block[bi].next);
  etdb_trie_push_block(trie, bi, head_out, !(*head_out)&&trie->block[bi].num);
}

static etdb_id_t
etdb_trie_find_place(etdb_trie_t *trie)
{
  etdb_id_t e;
  if(trie->block_head_close)  
    e   = trie->block[trie->block_head_close].ehead;
  else if(trie->block_head_open)   
    e   = trie->block[trie->block_head_open].ehead;
  else
    e   = etdb_trie_add_block(trie) << 8;
  assert(trie->node[e].check < 0 && trie->node[e].base < 0);
  return e;
}

static etdb_id_t
etdb_trie_find_place_interval(etdb_trie_t *trie, const uint8_t *first, const uint8_t *last)
{
  etdb_id_t bi = trie->block_head_open;
  if(bi){
    const etdb_id_t bz = trie->block[trie->block_head_open].prev;
    const int16_t nc   = (int16_t)(last - first + 1);
    while(1){
      etdb_trie_block_t *b = trie->block + bi;
      if(b->num >= nc && nc <= b->reject){
        etdb_id_t e = b->ehead;
        for(;;){
          const etdb_id_t base  = e ^ *first;
          const uint8_t *p      = first;
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
      const etdb_id_t bi_ = b->next;
      if(++b->trial == ETDB_TRIE_MAX_TRIAL)
        etdb_trie_transfer_block(trie, bi, &(trie->block_head_open), &(trie->block_head_close));
      if(bi == bz)  break;
      bi = bi_;
    }
  } 
  return etdb_trie_add_block(trie) << 8; 
}

static etdb_id_t
etdb_trie_pop_empty_node(etdb_trie_t *trie, const etdb_id_t base, const uint8_t label, const etdb_id_t from)
{
  const etdb_id_t e      = base < 0 ? etdb_trie_find_place(trie) : base ^ label;
  const etdb_id_t bi     = e >> 8;
  etdb_trie_node_t  *n = trie->node  + e;
  etdb_trie_block_t *b = trie->block + bi;

  if(--b->num == 0 && bi){
    etdb_trie_transfer_block(trie, bi, &(trie->block_head_close), &(trie->block_head_full));
  }else{
    trie->node[-n->base].check = n->check;
    trie->node[-n->check].base = n->base;
    if(e == b->ehead) 
      b->ehead = -n->check;
    if(bi && b->num == 1 && b->trial != ETDB_TRIE_MAX_TRIAL){ /*** Open to Closed***/
      etdb_trie_transfer_block(trie, bi, &(trie->block_head_open), &(trie->block_head_close));
    }
  }
  if(label)
    n->base  = -1;
  else
    n->value = 0;
  n->check   = from;
  if(base < 0)
    trie->node[from].base = e ^ label;

  return e;
}

static void
etdb_trie_push_empty_node(etdb_trie_t *trie, const etdb_id_t e)
{
  const etdb_id_t bi     = e >> 8;
  etdb_trie_block_t *b = trie->block + bi;
  if(++b->num == 1){ /*** full to closed ****/  
    b->ehead            = e;
    trie->node[e].base  = -e;
    trie->node[e].check = -e;
    if(bi)
      etdb_trie_transfer_block(trie, bi, &(trie->block_head_full), &(trie->block_head_close)); /*** full to closed ****/
  }else{
    const etdb_id_t prev       = b->ehead;
    const etdb_id_t next       = -(trie->node[prev].check);
    trie->node[e].base         = -prev;
    trie->node[e].check        = -next;
    trie->node[prev].check     = trie->node[next].base = -e;

    if(b->num == 2 || b->trial == ETDB_TRIE_MAX_TRIAL){ /*** closed to open ***/
      if(bi) etdb_trie_transfer_block(trie, bi, &(trie->block_head_close), &(trie->block_head_open));
    }
    b->trial = 0;
  }
  if(b->reject < trie->reject[b->num])
    b->reject = trie->reject[b->num];
  trie->ninfo[e].sibling = 0;  /*** reset ninfo; no child, no sibling ***/
  trie->ninfo[e].child   = 0;
}

static void
etdb_trie_push_sibling(etdb_trie_t *trie, etdb_id_t from, etdb_id_t base, const uint8_t label, int flag)
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
etdb_trie_pop_sibling(etdb_trie_t *trie, etdb_id_t from, etdb_id_t base, const uint8_t label)
{
  uint8_t *c = &(trie->ninfo[from].child);
  while(*c != label){
    c = &(trie->ninfo[base ^ *c].sibling);
  }
  *c = trie->ninfo[base ^ label].sibling;
}

static int
etdb_trie_consult(etdb_trie_t *trie, const etdb_id_t base_n, const etdb_id_t base_p, uint8_t c_n, uint8_t c_p)
{
  do{
    c_n = trie->ninfo[base_n ^ c_n].sibling;
    c_p = trie->ninfo[base_p ^ c_p].sibling;
  }while(c_n && c_p);
  return c_p;
}

static uint8_t*
etdb_trie_set_child(etdb_trie_t *trie, uint8_t *p, const etdb_id_t base, uint8_t c, const int label)
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

static etdb_id_t   /**** resolve conflict on base_n ^ label_n = base_p ^ label_p ***/
etdb_trie_resolve(etdb_trie_t *trie, etdb_id_t *from_n, const etdb_id_t base_n, const uint8_t label_n)
{
  etdb_id_t to_pn   = base_n ^ label_n;
  etdb_id_t from_p  = trie->node[to_pn].check;
  etdb_id_t base_p  = trie->node[from_p].base;

  int flag = etdb_trie_consult(trie, base_n, base_p, trie->ninfo[*from_n].child, trie->ninfo[from_p].child);
  uint8_t child[256];
  uint8_t* const first = &child[0];
  uint8_t* const last  = flag ? etdb_trie_set_child(trie, first, base_n, trie->ninfo[*from_n].child, label_n): 
                                etdb_trie_set_child(trie, first, base_p, trie->ninfo[from_p].child,  -1);
  etdb_id_t base = (first == last ? etdb_trie_find_place(trie) : etdb_trie_find_place_interval(trie, first, last)) ^ (*first);
  const etdb_id_t from = flag ? *from_n : from_p;
  const etdb_id_t base_ = flag ? base_n : base_p;
  const uint8_t *p;

  if(flag && *first == label_n)
    trie->ninfo[from].child = label_n;

  trie->node[from].base = base;

  for(p = first; p <= last; ++p){
    const etdb_id_t to  = etdb_trie_pop_empty_node(trie, base, *p, from);
    const etdb_id_t to_ = base_ ^ *p;
    trie->ninfo[to].sibling = (p == last ? 0 : *(p+1));
    if(flag && to_ == to_pn)    continue;
    etdb_trie_node_t *n  = trie->node + to;
    etdb_trie_node_t *n_ = trie->node + to_;

    if((n->base = n_->base) > 0 && *p)
    {
      uint8_t c    = trie->ninfo[to].child = trie->ninfo[to_].child;
      etdb_id_t nb = n->base;
      do{
        trie->node[nb ^ c].check = to;
      }while((c = trie->ninfo[nb ^ c].sibling));
    }

    if(!flag && to_ == *from_n){
      *from_n = to;
    }
    if(!flag && to_ == to_pn){
      etdb_trie_push_sibling(trie, *from_n, to_pn ^ label_n, label_n, 1);
      trie->ninfo[to_].child = 0;

      if(label_n) 
        n_->base = -1;
      else
        n_->value = 0;
      n_->check = *from_n;
    }else{
      etdb_trie_push_empty_node(trie, to_);
    }

    if(ETDB_TRIE_NUM_TRACKING_NODES){ /*** keep the traversed node updated**/
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

static etdb_id_t
etdb_trie_follow(etdb_trie_t *trie, etdb_id_t *from, const uint8_t label)
{
  etdb_id_t to         = 0;
  const etdb_id_t base = trie->node[*from].base;

  if(base < 0 || trie->node[to = base ^ label].check < 0){
    to = etdb_trie_pop_empty_node(trie, base, label, *from);
    etdb_trie_push_sibling(trie, *from, to ^ label, label, base >= 0);  
  }else if(trie->node[to].check != *from){
    to = etdb_trie_resolve(trie, from, base, label);
  } 
  return to;
}

static void
etdb_trie_update_entry(etdb_trie_t *trie, etdb_id_t *from, uint8_t word, uint8_t sp)
{
  /*** encode ***/
  if(!sp){
    if(word == '\0'){
      *from = etdb_trie_follow(trie, from, '%');
      etdb_trie_update_entry(trie, from, '0', 1);
    }else if(word == '%'){
      *from = etdb_trie_follow(trie, from, '%');
      etdb_trie_update_entry(trie, from, '%', 1);
    }else{
      *from  = etdb_trie_follow(trie, from, word);
    }
  }else{
    *from  = etdb_trie_follow(trie, from, word);
  }
}

etdb_id_t
etdb_trie_update(etdb_trie_t *trie, const char *key, size_t len, etdb_id_t value)
{
  etdb_id_t from = 0;
  etdb_id_t pos  = 0;
  if(len == 0)
    return -1;
  
  const uint8_t *key_uint8 = (const uint8_t*)key;
  int sp = 0;
  for( ; pos < len; ++(pos)){
    etdb_trie_update_entry(trie, &from, key_uint8[pos], 0);
  }

  const etdb_id_t to = etdb_trie_follow(trie, &from, 0);
  
  etdb_id_t p_value = trie->node[to].value;
  trie->node[to].value = value;
  return p_value;
}

static etdb_id_t
etdb_trie_find(etdb_trie_t *trie, const char *key, etdb_id_t *from, size_t pos, size_t len)
{
  etdb_id_t to;
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
    to         = trie->node[*from].base;
    to         = to ^ word;
    if(trie->node[to].check != *from)
      return ETDB_TRIE_NO_PATH;
    *from       = to;
    if(again_word != ' '){
      word = again_word;
      again_word = ' ';
      goto AGAIN;
    }
    ++pos;
  } 
  etdb_trie_node_t *n = trie->node + (trie->node[*from].base ^ 0);

  if(n->check != *from){
    return ETDB_TRIE_NO_VALUE;
  }
  return n->base;
}

etdb_id_t
etdb_trie_exact_match_search(etdb_trie_t *trie, const char *key, size_t len)
{
  size_t  pos  = 0;
  etdb_id_t from = 0;
  union{ etdb_id_t i; etdb_id_t value;} b;
  b.i = etdb_trie_find(trie, key, &from, pos, len);
  if(b.i == ETDB_TRIE_NO_PATH)
    b.i = ETDB_TRIE_NO_VALUE;
  return b.value;
}

static void
etdb_trie_common_prefix_search_dfs(etdb_trie_t *trie, etdb_id_t from, etdb_stack_t *stack_out)
{
  union{ etdb_id_t i; etdb_id_t value;}b;
  uint8_t child = 1;
 
  do{
    etdb_id_t l_from = from;
    b.i = etdb_trie_find(trie, (const char*)&child, &l_from, 0, 1);
    if(b.i != ETDB_TRIE_NO_PATH)
    {
      if(b.value != ETDB_TRIE_NO_VALUE){
        etdb_id_t *p = (etdb_id_t*)etdb_stack_push(stack_out);
        *p           = b.value;
      }
      etdb_trie_common_prefix_search_dfs(trie, l_from, stack_out);
    }
  }while(child++ != 255);
}

void
etdb_trie_common_prefix_search(etdb_trie_t *trie, const char *key, size_t len, etdb_stack_t *stack_result)
{
  size_t pos = 0;
  etdb_id_t from = 0;
  union{ etdb_id_t i; etdb_id_t value;} b;
  b.i = etdb_trie_find(trie, key, &from, pos, len);

  if(b.i == ETDB_TRIE_NO_PATH){
    return;
  }else{
    if(b.i != ETDB_TRIE_NO_VALUE){
      etdb_id_t *p = (etdb_id_t*)etdb_stack_push(stack_result);
      *p           = b.value;
    }
    etdb_trie_common_prefix_search_dfs(trie, from, stack_result); 
  }   
}

static void
etdb_trie_common_prefix_path_search_dfs(etdb_trie_t *trie, etdb_id_t from,  
                                        etdb_stack_t *stack_in, etdb_bytes_t *result, etdb_pool_t *pool)
{
  union{ etdb_id_t i; etdb_id_t value;}b;
  uint8_t child = 1;

  do{
    etdb_id_t l_from = from;
    b.i = etdb_trie_find(trie, (const char*)&child, &l_from, 0, 1);
    if(b.i != ETDB_TRIE_NO_PATH)
    {
      uint8_t *c     = (uint8_t*)etdb_stack_push(stack_in);
      *c             = child;

      if(b.value != ETDB_TRIE_NO_VALUE){
        etdb_bytes_t *new_bytes = etdb_palloc(pool, sizeof(etdb_bytes_t) + etdb_stack_size(stack_in));
        memcpy(new_bytes + 1, stack_in->elts, stack_in->nelts*stack_in->size);
        new_bytes->str.data     = (uint8_t*)(new_bytes + 1);
        new_bytes->str.len      = stack_in->nelts * stack_in->size;
        etdb_queue_insert_tail(&(result->queue), &(new_bytes->queue)); 
      }

      etdb_trie_common_prefix_path_search_dfs(trie, l_from, stack_in, result, pool);
      etdb_stack_pop(stack_in);
    }
  }while(child++ != 255);
}

int
etdb_trie_common_prefix_path_search(etdb_trie_t *trie, const char *key, size_t len, 
                                    etdb_stack_t *stack_in, etdb_bytes_t *result, etdb_pool_t *pool)
{
  int    ret     = 0;
  size_t pos     = 0;
  etdb_id_t from = 0;
  union{ etdb_id_t i; etdb_id_t value;} b;
  b.i  = etdb_trie_find(trie, key, &from, pos, len);

  if(b.i == ETDB_TRIE_NO_PATH){
    return;
  }else{
    if(b.i != ETDB_TRIE_NO_VALUE){
      ret            = 1;
    }
    etdb_trie_common_prefix_path_search_dfs(trie, from, stack_in, result, pool);
  }
  return ret;
}

static void
etdb_trie_erase_impl(etdb_trie_t *trie, etdb_id_t from)
{
  etdb_id_t e = trie->node[from].base ^ 0; 

  uint8_t flag = 0; /*** have sibling ***/
  do{
    etdb_trie_node_t *n   = trie->node + from;
    etdb_id_t nb          = n->base;

    flag = trie->ninfo[nb ^ trie->ninfo[from].child].sibling;
    if(flag){
      etdb_trie_pop_sibling(trie, from, nb, nb ^ e);
    }
    etdb_trie_push_empty_node(trie, e);
    e    = from;
    from = trie->node[from].check;
  }while(!flag);
}

etdb_id_t
etdb_trie_erase(etdb_trie_t *trie, const char *key, size_t len)
{
  etdb_id_t from = 0, pos = 0;
  union{ etdb_id_t i; etdb_id_t value;} b;

  b.i = etdb_trie_find(trie, key, &from, pos, len);
  if(b.i == ETDB_TRIE_NO_PATH || b.i == ETDB_TRIE_NO_VALUE){
    return -1;
  }
  etdb_trie_erase_impl(trie, from);
  return b.i;
}

size_t
etdb_trie_total_size(etdb_trie_t *trie)
{
  return trie->size;
}

size_t
etdb_trie_nonzero_size(etdb_trie_t *trie)
{
  size_t i = 0, to = 0;
  for(; to < trie->size; ++to){
    if(trie->node[to].check >= 0) ++i;
  }
  return i;
}

size_t
etdb_trie_num_keys(etdb_trie_t *trie)
{
  size_t i = 0, to = 0;
  for(; to < trie->size; ++to){
    if(trie->node[to].check >= 0 && trie->node[trie->node[to].check].base == to)
      ++i;
  }
  return i;
}