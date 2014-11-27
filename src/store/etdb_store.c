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

  int need_init   = etdb_store_file_exit(store->file);
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
