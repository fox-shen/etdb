#include <etdb_store_incs.h>

int 
etdb_init_lru(etdb_lru_t  *lru, etdb_log_t *log, int fd, uint32_t capacity)
{
  lru->capacity       = capacity;
  lru->size           = 0;
  lru->fd             = fd;
  lru->log            = log;
  lru->hash_head_size = lru->capacity/3;
  lru->pool           = etdb_create_pool(ETDB_MAX_ALLOC_FROM_POOL);

  lru->hash_head      = etdb_pcalloc(lru->pool, sizeof(etdb_lru_node_t)*lru->hash_head_size);  
  uint32_t pos;
  for(pos = 0; pos < lru->hash_head_size; ++pos){
    etdb_queue_init(&(lru->hash_head + pos)->hash_link);
    etdb_queue_init(&(lru->hash_head + pos)->lru_link);
  }
  etdb_queue_init(&(lru->lru_head.lru_link));
  return 0;
}

int 
etdb_destory_lru(etdb_lru_t *lru)
{
  etdb_destroy_pool(lru->pool);
}

uint8_t*
etdb_lru_fetch(etdb_lru_t *lru, etdb_store_id_t page_id)
{
  uint32_t hit_idx      = page_id % lru->hash_head_size;
  etdb_queue_t *head    = &(lru->hash_head + hit_idx)->hash_link;
  etdb_queue_t *next    = etdb_queue_next(head);

  for( ; next != head; next = etdb_queue_next(next) ){
    etdb_lru_node_t *n  = (etdb_lru_node_t*)next;
    if(n->page_id == page_id){ /*** hit page_id ***/
      etdb_queue_remove(&(n->lru_link));
      etdb_queue_insert_head(&(lru->lru_head.lru_link), &(n->lru_link));
      return n->page_value;
    }
  }
  etdb_lru_node_t *n    = NULL;
  int offset            = offsetof(etdb_lru_node_t, lru_link);
  if(lru->size == lru->capacity){  /*** maximum capacity reach ***/
    etdb_queue_t *top   = etdb_queue_prev(&(lru->lru_head.lru_link));
    etdb_queue_remove(top);
    n                   = (etdb_lru_node_t*)((uint8_t*)top - offset);     
    etdb_queue_remove(&(n->hash_link));
  }else{
    n                   = etdb_palloc(lru->pool, sizeof(etdb_lru_node_t));
    n->page_value       = etdb_palloc(lru->pool, ETDB_PAGE_SIZE);
    ++lru->size;
  } 
  n->page_id            = page_id;

  if(lseek(lru->fd, n->page_id*ETDB_PAGE_SIZE, SEEK_SET) < 0){
    etdb_log_print(lru->log, ETDB_LOG_ERR, "lseek failed %s %d", __FILE__, __LINE__);
  }
  if(read(lru->fd, n->page_value, ETDB_PAGE_SIZE) < 0){
    etdb_log_print(lru->log, ETDB_LOG_ERR, "read failed %s %d", __FILE__, __LINE__);
  }
  etdb_queue_insert_head(&(lru->lru_head.lru_link), &(n->lru_link));
  etdb_queue_insert_head(head, &(n->hash_link)); 

  return n->page_value;
}

uint8_t* 
etdb_lru_alloc_entry(etdb_lru_t *lru, etdb_store_id_t page_id)
{
  uint32_t hit_idx      = page_id % lru->hash_head_size;
  etdb_queue_t *head    = &(lru->hash_head + hit_idx)->hash_link;

  etdb_lru_node_t *n    = NULL;
  int offset            = offsetof(etdb_lru_node_t, lru_link);
  if(lru->size == lru->capacity){  /*** maximum capacity reach ***/
    etdb_queue_t *top   = etdb_queue_prev(&(lru->lru_head.lru_link));
    etdb_queue_remove(top);
    n                   = (etdb_lru_node_t*)((uint8_t*)top - offset);
    etdb_queue_remove(&(n->hash_link));
  }else{
    n                   = etdb_palloc(lru->pool, sizeof(etdb_lru_node_t));
    n->page_value       = etdb_palloc(lru->pool, ETDB_PAGE_SIZE);
    ++(lru->size);
  }
  n->page_id            = page_id;

  etdb_queue_insert_head(&(lru->lru_head.lru_link), &(n->lru_link));
  etdb_queue_insert_head(head, &(n->hash_link));

  return n->page_value;
}
