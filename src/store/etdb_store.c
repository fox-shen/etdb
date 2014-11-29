#include <etdb_store_incs.h>

typedef void (*etdb_index_add_handler)(etdb_store_t *store, void *value, etdb_store_id_t new_id);

static int
etdb_store_file_exit(const char *file_name)
{
  FILE *fp = fopen(file_name, "r");
  if(fp == NULL)   return 0;
  fclose(fp);
  return 1;
}

static void
etdb_store_flush_meta(etdb_store_t *store)
{
  int offset  = offsetof(etdb_store_t, node);
  if(lseek(store->fd, 0, SEEK_SET) < 0){
    etdb_log_print(store->log, ETDB_LOG_ERR, "lseek meta file failed %s %d", __FILE__, __LINE__);
  }
  if(write(store->fd, store, offset) < 0){
    etdb_log_print(store->log, ETDB_LOG_ERR, "write meta file failed %s %d", __FILE__, __LINE__);
  }
}

static void
etdb_store_load_meta(etdb_store_t *store)
{
  int offset = offsetof(etdb_store_t, node);
  if(lseek(store->fd, 0, SEEK_SET) < 0){
    etdb_log_print(store->log, ETDB_LOG_ERR, "lseek meta file failed %s %d", __FILE__, __LINE__);
  }
  if(read(store->fd, store, offset) < 0){
    etdb_log_print(store->log, ETDB_LOG_ERR, "write meta file failed %s %d", __FILE__, __LINE__);
  }
}

static void
etdb_store_write(etdb_store_t *store, int fd, etdb_store_id_t page_id, void *data)
{
  if(lseek(fd, page_id*ETDB_PAGE_SIZE, SEEK_SET) <  0){
    etdb_log_print(store->log, ETDB_LOG_ERR, "lseek failed %s %d", __FILE__, __LINE__);
  }
  if(write(fd, data, ETDB_PAGE_SIZE) < 0){
    etdb_log_print(store->log, ETDB_LOG_ERR, "write failed %s %d", __FILE__, __LINE__);
  }
}

static etdb_index_node_t*
etdb_store_index_node_at(etdb_store_t *store, etdb_store_id_t id)
{
  etdb_store_id_t page_id  = id/ETDB_INDEX_NODE_SIZE_PER_PAGE;
  etdb_store_id_t offset   = id%ETDB_INDEX_NODE_SIZE_PER_PAGE;
 
  etdb_index_node_t *nodes = (etdb_index_node_t*)etdb_lru_fetch(&store->node.lru, page_id); 
  return nodes + offset;
}

static etdb_index_ninfo_t*
etdb_store_index_ninfo_at(etdb_store_t *store, etdb_store_id_t id)
{
  etdb_store_id_t page_id    = id/ETDB_INDEX_NINFO_SIZE_PER_PAGE;
  etdb_store_id_t offset     = id%ETDB_INDEX_NINFO_SIZE_PER_PAGE;

  etdb_index_ninfo_t *ninfos = (etdb_index_ninfo_t*)etdb_lru_fetch(&store->ninfo.lru, page_id);
  return ninfos + offset;  
}

static etdb_index_block_t*
etdb_store_index_block_at(etdb_store_t *store, etdb_store_id_t id)
{
  etdb_store_id_t page_id    = id/ETDB_INDEX_BLOCK_SIZE_PER_PAGE;
  etdb_store_id_t offset     = id%ETDB_INDEX_BLOCK_SIZE_PER_PAGE;

  etdb_index_block_t *blocks = (etdb_index_block_t*)etdb_lru_fetch(&store->block.lru, page_id);
  return blocks + offset;
}

static void
etdb_index_node_init(etdb_store_t *store, etdb_index_node_t *node)
{
  int i                   = 0;
  node[i].base            = 0;
  node[i].check           = -1;

  for(i = 1; i < 256; ++i){
    if(i == 1){
      node[i].base        = -255;
      node[i].check       = -(i + 1);
    }else if(i == 255){
      node[i].base        = -(i - 1);
      node[i].check       = -1;
    }else{
      node[i].base        = -(i - 1);
      node[i].check       = -(i + 1);
    }
  }
}

static etdb_index_node_t*
etdb_store_add_node(etdb_store_t *store, etdb_store_id_t *new_id)
{
  *new_id                   = store->capacity/ETDB_INDEX_NODE_SIZE_PER_PAGE;
  etdb_index_node_t* value  = (etdb_index_node_t*)etdb_lru_alloc_entry(&store->node.lru, *new_id);
  memset(value, 0, sizeof(etdb_index_node_t)*ETDB_INDEX_NODE_SIZE_PER_PAGE); 
  return value;
}

static etdb_index_ninfo_t*
etdb_store_add_ninfo(etdb_store_t *store, etdb_store_id_t *new_id)
{
  *new_id                   = store->capacity/ETDB_INDEX_NINFO_SIZE_PER_PAGE;
  etdb_index_ninfo_t *value = (etdb_index_ninfo_t*)etdb_lru_alloc_entry(&store->ninfo.lru, *new_id);
  memset(value, 0, sizeof(etdb_index_ninfo_t)*ETDB_INDEX_NINFO_SIZE_PER_PAGE);
  return value;
}

static etdb_index_block_t*
etdb_store_add_block(etdb_store_t *store, etdb_store_id_t *new_id)
{
  *new_id                   = (store->capacity/ETDB_INDEX_BLOCK_SIZE_PER_PAGE) >> 8;
  etdb_index_block_t *value = (etdb_index_block_t*)etdb_lru_alloc_entry(&store->block.lru, *new_id);
  int i;
  for(i = 0; i < ETDB_INDEX_BLOCK_SIZE_PER_PAGE; ++i){
    value[i].prev   = value[i].next = 0;
    value[i].num    = 256;
    value[i].reject = 257;
    value[i].trial  = 0;
    value[i].ehead  = 0;
  } 
  return value;
}

int 
etdb_init_store(etdb_store_t *store, etdb_log_t *log, etdb_store_option_t *option)
{
  int i;
  memset(store, 0, sizeof(etdb_store_t));

  sprintf(store->node.file,  "%s/node",  option->dir);
  sprintf(store->ninfo.file, "%s/ninfo", option->dir);
  sprintf(store->block.file, "%s/block", option->dir);
  sprintf(store->file,       "%s/meta",  option->dir);
  store->log      = log;

  int need_init   = !etdb_store_file_exit(store->file);
  store->fd       = open(store->file, O_RDWR | O_CREAT | O_APPEND , 0644); 
  store->node.fd  = open(store->node.file,  O_RDWR | O_CREAT | O_APPEND , 0644);
  store->ninfo.fd = open(store->ninfo.file, O_RDWR | O_CREAT | O_APPEND , 0644);
  store->block.fd = open(store->block.file, O_RDWR | O_CREAT | O_APPEND , 0644);

  etdb_init_lru(&store->node.lru,  log, store->node.fd,  option->index_node_lru_cap);
  etdb_init_lru(&store->ninfo.lru, log, store->ninfo.fd, option->index_ninfo_lru_cap);
  etdb_init_lru(&store->block.lru, log, store->block.fd, option->index_block_lru_cap);
  
  if(need_init){
     for(i = 0; i <= 256; ++i)  store->reject[i] = i + 1;
     store->capacity = ETDB_INDEX_NODE_SIZE_PER_PAGE;
     store->size     = 256;
     etdb_store_flush_meta(store); 

     etdb_store_id_t new_id;
     etdb_index_node_t *node_ptr = etdb_store_add_node(store, &new_id);
     etdb_index_node_init(store, node_ptr);     
     etdb_store_write(store, store->node.fd, new_id, node_ptr);   

     etdb_index_ninfo_t *ninfo_ptr = etdb_store_add_ninfo(store, &new_id);
     etdb_store_write(store, store->ninfo.fd, new_id, ninfo_ptr);

     etdb_index_block_t *block_ptr = etdb_store_add_block(store, &new_id);
     block_ptr[0].ehead            = 1;
     etdb_store_write(store, store->block.fd, new_id, block_ptr);
  }else{
     etdb_store_load_meta(store);    
  }
}

static void
etdb_store_pop_block(etdb_store_t *store, const etdb_store_id_t bi, etdb_store_id_t *head_in, int last)
{
  if(last){
    *head_in  = 0;
  }else{
    etdb_index_block_t *block = etdb_store_index_block_at(store, bi);
    etdb_index_block_t *block_prev = etdb_store_index_block_at(store, block->prev);
    etdb_index_block_t *block_next = etdb_store_index_block_at(store, block->next);
   
    block_prev->next   = block->next;
    block_next->prev   = block->prev;
    if(bi == *head_in)   *head_in = block->next;
  }
}

static void
etdb_store_push_block(etdb_store_t *store, const etdb_store_id_t bi, etdb_store_id_t *head_out, int empty)
{
  etdb_index_block_t *block = etdb_store_index_block_at(store, bi);
  if(empty){
    *head_out  = block->prev  = block->next = bi;
  }else{
    etdb_index_block_t *block_head_out = etdb_store_index_block_at(store, *head_out);
    etdb_index_block_t *block_head_out_prev = etdb_store_index_block_at(store, block_head_out->prev);
    block->prev = block_head_out->prev;
    block->next = *head_out;
    *head_out   = block_head_out->prev = block_head_out_prev->next = bi;
  }
}

static etdb_store_id_t
etdb_store_alloc_block(etdb_store_t *store)
{
  etdb_store_id_t i, new_id;
  etdb_index_node_t  *node  = NULL;
  etdb_index_ninfo_t *ninfo = NULL;
  etdb_index_block_t *block = NULL;

  if(store->size == store->capacity){
    node              = etdb_store_add_node(store, &new_id);
    store->capacity  += ETDB_INDEX_NODE_SIZE_PER_PAGE;
  }
}

static void
etdb_store_transfer_block(etdb_store_t *store, const etdb_store_id_t bi, etdb_store_id_t *head_in, etdb_store_id_t *head_out)
{
  etdb_index_block_t *block = etdb_store_index_block_at(store, bi);
  etdb_store_pop_block(store,  bi, head_in,  bi == block->next);
  etdb_store_push_block(store, bi, head_out, !(*head_out)&&block->num);
}

static etdb_store_id_t
etdb_store_find_place(etdb_store_t *store)
{
  etdb_store_id_t e;
  if(store->block_head_close){
    etdb_index_block_t *block = etdb_store_index_block_at(store, store->block_head_close);
    e = block->ehead;
  }else if(store->block_head_open){
    etdb_index_block_t *block = etdb_store_index_block_at(store, store->block_head_open);
    e = block->ehead;
  }else{
    e = etdb_store_alloc_block(store) << 8;
  }
  return e;
}

static etdb_store_id_t
etdb_store_pop_empty_node(etdb_store_t *store, const etdb_store_id_t base, const uint8_t label, const etdb_store_id_t from)
{
  const etdb_store_id_t e  = base < 0 ? etdb_store_find_place(store) : base ^ label;
  const etdb_store_id_t bi = e >> 8;
  etdb_index_node_t  *n    = etdb_store_index_node_at(store,  e);
  etdb_index_block_t *b    = etdb_store_index_block_at(store, bi);

  if(--b->num == 0 && bi){
    etdb_store_transfer_block(store, bi, &(store->block_head_close), &(store->block_head_full));
  }else{
    etdb_index_node_t *n_base  = etdb_store_index_node_at(store, -n->base);
    etdb_index_node_t *n_check = etdb_store_index_node_at(store, -n->check);
    n_base->check              = n->check;
    n_check->base              = n->base;

    if(e == b->ehead)
      b->ehead = -n->check;
    if(bi && b->num == 1 && b->trial != ETDB_STORE_INDEX_MAX_TRIAL){
      etdb_store_transfer_block(store, bi, &(store->block_head_open), &(store->block_head_close));
    }
  }
  if(label)   n->base  = -1;
  else        n->value = 0;

  n->check             = from;
  if(base < 0){
    etdb_index_node_t *n_from = etdb_store_index_node_at(store, from);
    n_from->base              = e ^ label;
  }
  return e;  
}

static void
etdb_store_push_sibling(etdb_store_t *store, etdb_store_id_t from, etdb_store_id_t base, const uint8_t label, int flag)
{
  etdb_index_ninfo_t *ninfo = etdb_store_index_ninfo_at(store, from);
  uint8_t c =  ninfo->child;
  if(flag && !(c)){
    ninfo = etdb_store_index_ninfo_at(store, base ^ c);
    c     = ninfo->sibling;
    ninfo->sibling = label;
  }else{
    ninfo->child = label;
  }
  ninfo = etdb_store_index_ninfo_at(store, base ^ label);
  ninfo->sibling = c;
}

static void
etdb_store_pop_sibling(etdb_store_t *store, etdb_store_id_t from, etdb_store_id_t base, const uint8_t label)
{
  etdb_index_ninfo_t *nvalue = etdb_store_index_ninfo_at(store, base ^ label);
  uint8_t sibling   = nvalue->sibling;

  etdb_index_ninfo_t *ninfo  = etdb_store_index_ninfo_at(store, from);
  uint8_t *c  = &ninfo->child;
  while(*c != label){
    ninfo = etdb_store_index_ninfo_at(store, base ^ (*c));
    c     = &ninfo->sibling;
  }
  *c = sibling;  
}

static int
etdb_store_consult(etdb_store_t *store, const etdb_store_id_t base_n, const etdb_store_id_t base_p, uint8_t c_n, uint8_t c_p)
{
  do{
    etdb_index_ninfo_t *c_n_info = etdb_store_index_ninfo_at(store, base_n ^ c_n);
    c_n = c_n_info->sibling;
    etdb_index_ninfo_t *c_p_info = etdb_store_index_ninfo_at(store, base_p ^ c_p);
    c_p = c_p_info->sibling; 
  }while(c_n && c_p);
  return c_p;
}

static uint8_t*
etdb_store_set_child(etdb_store_t *store, uint8_t *p, const etdb_store_id_t base, uint8_t c, const int label)
{
  --p;
  if(!c){
    *++p = c;
    etdb_index_ninfo_t *ninfo = etdb_store_index_ninfo_at(store, base ^ c);
    c = ninfo->sibling;
  }
  if(label != -1){
    *++p = (uint8_t)label;
  }
  while(c){
    *++p = c;
    etdb_index_ninfo_t *ninfo = etdb_store_index_ninfo_at(store, base ^ c);
    c  = ninfo->sibling;
  }
  return p;
}

static etdb_store_id_t  /*** resolve conflict on base_n ^ label_n = base_p ^ label_p ****/
etdb_store_resolve(etdb_store_t *store, etdb_store_id_t *from_n, const etdb_store_id_t base_n, const uint8_t label_n)
{
/*
  etdb_store_id_t to_pn          = base_n ^ label_n;
  etdb_index_node_t *node_to_pn  = etdb_store_index_node_at(store, to_pn);
  etdb_store_id_t from_p         = node_to_pn->check;
  etdb_index_node_t *node_from_p = etdb_store_index_node_at(store, from_p);
  etdb_store_id_t base_p         = node_from_p->base;

  etdb_index_ninfo_t *ninfo_from_n = etdb_store_index_ninfo_at(store, *from_n);
  etdb_index_ninfo_t *ninfo_from_p = etdb_store_index_ninfo_at(store, from_p);

  int flag = etdb_store_consult(store, base_n, base_p, ninfo_from_n->child, ninfo_from_p->child);
  uint8_t child[256];
  uint8_t* const firt = &child[0];
  uint8_t* const last = flag ? etdb_store_set_child(store, first, base_n, ninfo_from_n->child, label_n): etdb_store_set_child(trie, first, base_p, ninfo_from_p->child, -1);

  etdb_store_id_t base = (first == last ? etdb_store_find_place(store) : etdb_store_find_place_interval(store, first, last)) ^ (*first);
  const etdb_store_id_t  from    = flag ? *from_n : from_p;
  const etdb_store_id_t  base_   = flag ?  base_n : base_p;
  const uint8_t *p;

  if(flag && *first == label_n){
   
  }
                             
  */
   
}

static etdb_store_id_t
etdb_store_follow(etdb_store_t *store, etdb_store_id_t *from, const uint8_t label)
{
  etdb_store_id_t  to     = 0;
  etdb_index_node_t *node = etdb_store_index_node_at(store, *from);
  const etdb_store_id_t base = node->base;

  if(base < 0 ){
    to   = etdb_store_pop_empty_node(store, base, label, *from);
    etdb_store_push_sibling(store, *from, to ^ label, label, base >= 0);   
  }else{
    to   = base ^ label;
    node = etdb_store_index_node_at(store, to);
    if(node->check < 0){
      to   = etdb_store_pop_empty_node(store, base, label, *from);
      etdb_store_push_sibling(store, *from, to ^ label, label, base >= 0);
    }else{
      to   = etdb_store_resolve(store, from, base, label);
    }
  }
  return to;
}

static void
etdb_store_update_entry(etdb_store_t *store, etdb_store_id_t *from, uint8_t word, uint8_t sp)
{
  if(!sp){
    if(word == '\0'){
      *from = etdb_store_follow(store, from, '%');
      etdb_store_update_entry(store, from, '0', 1);
    }else if(word == '%'){
      *from = etdb_store_follow(store, from, '%');
      etdb_store_update_entry(store, from, '%', 1);
    }else{
      *from = etdb_store_follow(store, from, word);
    }
  }else{
    *from = etdb_store_follow(store, from, word);
  }
}

int 
etdb_store_update(etdb_store_t *store, uint8_t *key, uint32_t kl, uint8_t *value, uint32_t vl)
{
  etdb_store_id_t from = 0, pos = 0;
  if(kl == 0)  return -1;

  for(; pos < kl; ++pos){
    etdb_store_update_entry(store, &from, key[pos], 0);
  }
  const etdb_store_id_t to  = etdb_store_follow(store, &from, 0);
   
}
