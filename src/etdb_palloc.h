#ifndef H_ETDB_PALLOC_H
#define H_ETDB_PALLOC_H

#define ETDB_MAX_ALLOC_FROM_POOL 4096
#ifndef ETDB_ALIGNMENT
#define ETDB_ALIGNMENT sizeof(unsigned long)
#endif
#define etdb_align_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))
#define ETDB_POOL_ALIGNMENT       16

typedef struct etdb_pool_large_s etdb_pool_large_t;
struct etdb_pool_large_s{
  etdb_pool_large_t *next;
  void              *alloc;
};

typedef struct etdb_pool_data_s etdb_pool_data_t;
typedef struct etdb_pool_s etdb_pool_t;
struct etdb_pool_data_s{
  uint8_t           *last;
  uint8_t           *end;
  etdb_pool_t       *next;
  size_t            failed;
};

struct etdb_pool_s{
  etdb_pool_data_t   d;
  size_t             max;
  etdb_pool_t        *current;
  etdb_pool_large_t  *large;
};

etdb_pool_t* etdb_create_pool(size_t size);
void etdb_destroy_pool(etdb_pool_t *pool);

void* etdb_palloc(etdb_pool_t *pool, size_t size);
void* etdb_pnalloc(etdb_pool_t *pool, size_t size);
void* etdb_pcalloc(etdb_pool_t *pool, size_t size);

int etdb_pfree(etdb_pool_t *pool, void *p);
void* etdb_palloc_large(etdb_pool_t *pool, size_t size);
void* etdb_prealloc_large(etdb_pool_t *pool, void *raw, size_t size);

void* etdb_palloc_temp(etdb_pool_t *pool, size_t size);

#endif
