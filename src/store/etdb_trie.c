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

etdb_trie_node_t*   
NodeAt(etdb_trie_t  *trie, etdb_id_t idx)
{
  return trie->node + idx;
}

etdb_trie_ninfo_t*  
NinfoAt(etdb_trie_t *trie, etdb_id_t idx)
{
  return trie->ninfo + idx;
}

etdb_trie_block_t*  
BlockAt(etdb_trie_t *trie, etdb_id_t idx)
{
  return trie->block + idx;
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

  trie->capacity      = trie->size = 256;
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
    const etdb_trie_block_t *b = BlockAt(trie, bi);
    BlockAt(trie, b->prev)->next  = b->next;
    BlockAt(trie, b->next)->prev = b->prev;
    if(bi == *head_in) 
      *head_in = b->next;  
  }
}

static void
etdb_trie_push_block(etdb_trie_t *trie, const etdb_id_t bi, etdb_id_t *head_out, int empty)
{
  etdb_trie_block_t *b  = BlockAt(trie, bi);
  if(empty){
    *head_out = b->prev = b->next = bi;
  }else{
    etdb_id_t *tail_out   = &(BlockAt(trie, *head_out)->prev);
    b->prev               = *tail_out;
    b->next               = *head_out;

    BlockAt(trie, *tail_out)->next = bi;
    *head_out = *tail_out =  bi;
  }
}

static etdb_id_t
etdb_trie_add_block(etdb_trie_t *trie)
{
  etdb_id_t i;
  if(trie->size == trie->capacity)
  {
    etdb_id_t old_cap     = trie->capacity;
    trie->capacity        = 2*trie->capacity;
  
    etdb_trie_node_realloc(&(trie->node),   trie->capacity,      old_cap);
    etdb_trie_ninfo_realloc(&(trie->ninfo), trie->capacity,      old_cap);
    etdb_trie_block_realloc(&(trie->block), trie->capacity >> 8, old_cap >> 8);
  }

  BlockAt(trie, trie->size >> 8)->ehead   = trie->size;
  NodeAt(trie, trie->size)->base          = -(trie->size + 255);
  NodeAt(trie, trie->size)->check         = -(trie->size + 1);

  for(i = trie->size + 1; i < trie->size + 255; ++i){
    NodeAt(trie, i)->base   = -(i - 1);
    NodeAt(trie, i)->check  = -(i + 1);
  }
  NodeAt(trie, trie->size + 255)->base  = -(trie->size + 254);
  NodeAt(trie, trie->size + 255)->check = -(trie->size);

  etdb_trie_push_block(trie, trie->size >> 8, &(trie->block_head_open), !trie->block_head_open);
  trie->size += 256;
  return (trie->size >> 8) - 1; 
}

static void
etdb_trie_transfer_block(etdb_trie_t *trie, const etdb_id_t bi, etdb_id_t *head_in, etdb_id_t *head_out)
{
  etdb_trie_pop_block (trie, bi, head_in,  bi == BlockAt(trie, bi)->next);
  etdb_trie_push_block(trie, bi, head_out, !(*head_out)&& BlockAt(trie, bi)->num);
}



/*** should transfer data here ****/
static etdb_id_t
etdb_trie_find_place(etdb_trie_t *trie)
{
  etdb_id_t e;
  if(trie->block_head_close)  
    e   = BlockAt(trie, trie->block_head_close)->ehead;
  else if(trie->block_head_open)   
    e   = BlockAt(trie, trie->block_head_open)->ehead;
  else
    e   = etdb_trie_add_block(trie) << 8;
  return e;
}

static etdb_id_t
etdb_trie_find_place_interval(etdb_trie_t *trie, const uint8_t *first, const uint8_t *last)
{
  etdb_id_t bi = trie->block_head_open;
  if(bi){
    const etdb_id_t bz = BlockAt(trie, trie->block_head_open)->prev;
    const int16_t nc   = (int16_t)(last - first + 1);
    while(1){
      etdb_trie_block_t *b = BlockAt(trie, bi);
      if(b->num >= nc && nc <= b->reject){
        etdb_id_t e = b->ehead;
        for(;;){
          const etdb_id_t base  = e ^ *first;
          const uint8_t *p      = first;
          for(; NodeAt(trie, base ^ *++p)->check  < 0; ){
            if(p == last)  return b->ehead = e;
          }
          if((e = -NodeAt(trie, e)->check) == b->ehead) 
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
  etdb_trie_node_t  *n = NodeAt(trie, e);
  etdb_trie_block_t *b = BlockAt(trie, bi);

  if(--b->num == 0 && bi){
    etdb_trie_transfer_block(trie, bi, &(trie->block_head_close), &(trie->block_head_full));
  }else{
    NodeAt(trie, -n->base)->check = n->check;
    NodeAt(trie, -n->check)->base = n->base;
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
    NodeAt(trie, from)->base = e ^ label;

  return e;
}

static void
etdb_trie_push_empty_node(etdb_trie_t *trie, const etdb_id_t e)
{
  const etdb_id_t bi    = e >> 8;
  etdb_trie_block_t *b  = BlockAt(trie, bi);
  if(++b->num == 1){ /*** full to closed ****/  
    b->ehead            = e;
    NodeAt(trie, e)->base  = -e;
    NodeAt(trie, e)->check = -e;
    if(bi)
      etdb_trie_transfer_block(trie, bi, &(trie->block_head_full), &(trie->block_head_close)); 
    /*** full to closed ****/
  }else{
    const etdb_id_t prev       = b->ehead;
    const etdb_id_t next       = -(NodeAt(trie, prev)->check);
    NodeAt(trie, e)->base      = -prev;
    NodeAt(trie, e)->check     = -next;
    NodeAt(trie, prev)->check  = NodeAt(trie, next)->base  = -e;

    if(b->num == 2 || b->trial == ETDB_TRIE_MAX_TRIAL){ /*** closed to open ***/
      if(bi) etdb_trie_transfer_block(trie, bi, &(trie->block_head_close), &(trie->block_head_open));
    }
    b->trial = 0;
  }
  if(b->reject < trie->reject[b->num])
    b->reject = trie->reject[b->num];

  NinfoAt(trie, e)->sibling = 0; /*** reset ninfo; no child, no sibling ***/
  NinfoAt(trie, e)->child   = 0;
}

static void
etdb_trie_push_sibling(etdb_trie_t *trie, etdb_id_t from, etdb_id_t base, const uint8_t label, int flag)
{
  uint8_t* c = &(NinfoAt(trie, from)->child);
  if(flag && /*label > *c */ !(*c)){
    do{
      c = &(NinfoAt(trie, base ^ *c)->sibling);
    }while(/**c && *c < label*/0);
  }
  NinfoAt(trie, base ^ label)->sibling = *c;
  *c = label;
}

static void
etdb_trie_pop_sibling(etdb_trie_t *trie, etdb_id_t from, etdb_id_t base, const uint8_t label)
{
  uint8_t *c = &(NinfoAt(trie, from)->child);
  while(*c != label){
    c = &(NinfoAt(trie, base ^ *c)->sibling);
  }
  *c = NinfoAt(trie, base ^ label)->sibling;
}

static int
etdb_trie_consult(etdb_trie_t *trie, const etdb_id_t base_n, const etdb_id_t base_p, uint8_t c_n, uint8_t c_p)
{
  do{
    c_n = NinfoAt(trie, base_n ^ c_n)->sibling;
    c_p = NinfoAt(trie, base_p ^ c_p)->sibling;
  }while(c_n && c_p);
  return c_p;
}

static uint8_t*
etdb_trie_set_child(etdb_trie_t *trie, uint8_t *p, const etdb_id_t base, uint8_t c, const int label)
{
  --p;
  if(!c){
    *++p = c;
    c = NinfoAt(trie, base ^ c)->sibling;
  }
  if(label != -1){
    *++p = (uint8_t)label;
  }
  while(c){
    *++p = c;
    c = NinfoAt(trie, base ^ c)->sibling;
  }
  return p;
}

static etdb_id_t   /**** resolve conflict on base_n ^ label_n = base_p ^ label_p ***/
etdb_trie_resolve(etdb_trie_t *trie, etdb_id_t *from_n, const etdb_id_t base_n, const uint8_t label_n)
{
  etdb_id_t to_pn   = base_n ^ label_n;
  etdb_id_t from_p  = NodeAt(trie, to_pn)->check;
  etdb_id_t base_p  = NodeAt(trie, from_p)->base;

  int flag = etdb_trie_consult(trie, base_n, base_p, NinfoAt(trie, *from_n)->child, NinfoAt(trie, from_p)->child);
  uint8_t child[256];
  uint8_t* const first = &child[0];
  uint8_t* const last  = flag ? etdb_trie_set_child(trie, first, base_n, NinfoAt(trie, *from_n)->child, label_n): 
                                etdb_trie_set_child(trie, first, base_p, NinfoAt(trie, from_p)->child,  -1);
  etdb_id_t base = (first == last ? etdb_trie_find_place(trie) : etdb_trie_find_place_interval(trie, first, last)) ^ (*first);
  const etdb_id_t from = flag ? *from_n : from_p;
  const etdb_id_t base_ = flag ? base_n : base_p;
  const uint8_t *p;

  if(flag && *first == label_n)
  {
    NinfoAt(trie, from)->child = label_n;
  }
  NodeAt(trie, from)->base = base;

  for(p = first; p <= last; ++p){
    const etdb_id_t to  = etdb_trie_pop_empty_node(trie, base, *p, from);
    const etdb_id_t to_ = base_ ^ *p;
    NinfoAt(trie, to)->sibling = (p == last ? 0 : *(p+1));
    if(flag && to_ == to_pn)    continue;
    etdb_trie_node_t *n  = NodeAt(trie, to);
    etdb_trie_node_t *n_ = NodeAt(trie, to_);

    if((n->base = n_->base) > 0 && *p)
    {
      uint8_t c    = NinfoAt(trie, to)->child = NinfoAt(trie, to_)->child;
      etdb_id_t nb = n->base;
      do{
        NodeAt(trie, nb ^ c)->check = to;
      }while((c = NinfoAt(trie, nb ^ c)->sibling));
    }

    if(!flag && to_ == *from_n){
      *from_n = to;
    }
    if(!flag && to_ == to_pn){
      etdb_trie_push_sibling(trie, *from_n, to_pn ^ label_n, label_n, 1);
      NinfoAt(trie, to_)->child = 0;

      if(label_n) 
        n_->base = -1;
      else
        n_->value = 0;
      n_->check = *from_n;
    }else{
      etdb_trie_push_empty_node(trie, to_);
    }
  }  
  return flag ? base ^ label_n : to_pn;
}

static etdb_id_t
etdb_trie_follow(etdb_trie_t *trie, etdb_id_t *from, const uint8_t label)
{
  etdb_id_t to         = 0;
  const etdb_id_t base = NodeAt(trie, *from)->base;

  if(base < 0 || NodeAt(trie, to = base ^ label)->check < 0){
    to = etdb_trie_pop_empty_node(trie, base, label, *from);
    etdb_trie_push_sibling(trie, *from, to ^ label, label, base >= 0);  
  }else if(NodeAt(trie, to)->check != *from){
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
  for( ; pos < len; ++(pos)){
    etdb_trie_update_entry(trie, &from, key_uint8[pos], 0);
  }

  const etdb_id_t to      = etdb_trie_follow(trie, &from, 0); 
  etdb_id_t p_value       = NodeAt(trie, to)->value;

  NodeAt(trie, to)->value = value;
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
    to         = NodeAt(trie, *from)->base;
    to         = to ^ word;
    if(NodeAt(trie, to)->check != *from)
      return ETDB_TRIE_NO_PATH;
    *from       = to;
    if(again_word != ' '){
      word = again_word;
      again_word = ' ';
      goto AGAIN;
    }
    ++pos;
  } 
  etdb_trie_node_t *n = NodeAt(trie, NodeAt(trie, *from)->base ^ 0);
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

etdb_id_t
etdb_trie_match_longest_search(etdb_trie_t *trie, const char *key, size_t len, size_t *match_len)
{
  etdb_id_t to, from = 0, ret = ETDB_TRIE_NO_PATH;
  uint8_t *key_int8 = (uint8_t*)key;
  size_t pos = 0;
  

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
    to         = NodeAt(trie, from)->base;
    to         = to ^ word;
    if(NodeAt(trie, to)->check != from)
      return ret;
    else{
      etdb_trie_node_t *n = NodeAt(trie, NodeAt(trie, to)->base ^ 0);
      if(n->check == to){
        ret        = n->value;
        *match_len = pos + 1;
      }
    }
    from       = to;
    if(again_word != ' '){
      word       = again_word;
      again_word = ' ';
      goto AGAIN;
    }
    ++pos;
  }
  return ret; 
}

static void
etdb_trie_common_prefix_search_dfs(etdb_trie_t *trie, etdb_id_t from, etdb_stack_t *stack_out)
{
  union{ etdb_id_t i; etdb_id_t value;}b;
  etdb_id_t base = NodeAt(trie, from)->base;
  uint8_t child  = NinfoAt(trie, from)->child;
  if(child == 0){
    child = NinfoAt(trie, base ^ child)->sibling;
  }
 
  while(child != 0)
  {
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
    child = NinfoAt(trie, base ^ child)->sibling;
  }
}

void
etdb_trie_common_prefix_search(etdb_trie_t *trie, const char *key, size_t len, etdb_stack_t *stack_result)
{
  size_t pos = 0;
  etdb_id_t from = 0, to = 0;
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
  uint8_t child  = NinfoAt(trie, from)->child;
  etdb_id_t base = NodeAt(trie, from)->base;
  if(child == 0){
    child = NinfoAt(trie, base ^ child)->sibling;
  }

  while(child != 0)
  {
    etdb_id_t l_from = from;
    b.i = etdb_trie_find(trie, (const char*)&child, &l_from, 0, 1);
    if(b.i != ETDB_TRIE_NO_PATH)
    {
      uint8_t *c     = (uint8_t*)etdb_stack_push(stack_in);
      *c             = child;

      if(b.value != ETDB_TRIE_NO_VALUE){
        etdb_bytes_t *new_bytes = etdb_palloc(pool, sizeof(etdb_bytes_t) + etdb_stack_size(stack_in) + sizeof(b.value));
        memcpy(new_bytes + 1, stack_in->elts, stack_in->nelts*stack_in->size);
        memcpy((uint8_t*)new_bytes + sizeof(etdb_bytes_t) + etdb_stack_size(stack_in), &(b.value), sizeof(b.value));
        new_bytes->str.data     = (uint8_t*)(new_bytes + 1);
        new_bytes->str.len      = stack_in->nelts * stack_in->size;
        etdb_queue_insert_tail(&(result->queue), &(new_bytes->queue)); 
      }

      etdb_trie_common_prefix_path_search_dfs(trie, l_from, stack_in, result, pool);
      etdb_stack_pop(stack_in);
    }
    child = NinfoAt(trie, base ^ child)->sibling;
  }
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
  etdb_id_t e = NodeAt(trie, from)->base ^ 0;

  uint8_t flag = 0; /*** have sibling ***/
  do{
    etdb_trie_node_t *n   = NodeAt(trie, from);
    etdb_id_t nb          = n->base;

    flag = NinfoAt(trie, nb ^ NinfoAt(trie, from)->child)->sibling;
    if(flag){
      etdb_trie_pop_sibling(trie, from, nb, nb ^ e);
    }
    etdb_trie_push_empty_node(trie, e);
    e    = from;
    from = NodeAt(trie, from)->check;
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
    if(NodeAt(trie, to)->check  >= 0) ++i;
  }
  return i;
}

size_t
etdb_trie_num_keys(etdb_trie_t *trie)
{
  size_t i = 0, to = 0;
  for(; to < trie->size; ++to){
    if(NodeAt(trie, to)->check  >= 0 && NodeAt(trie, NodeAt(trie, to)->check)->base == to)
      ++i;
  }
  return i;
}
