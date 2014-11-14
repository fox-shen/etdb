#ifndef H_EMDB_PALLOC_H
#define H_EMDB_PALLOC_H

#define EMDB_MAX_ALLOC_FROM_POOL 4096
#ifndef EMDB_ALIGNMENT
#define EMDB_ALIGNMENT sizeof(unsigned long)
#endif
#define emdb_align_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))
#define EMDB_POOL_ALIGNMENT       16

typedef struct emdb_pool_large_s emdb_pool_large_t;
struct emdb_pool_large_s{
  emdb_pool_large_t *next;
  void              *alloc;
};

typedef struct emdb_pool_data_s emdb_pool_data_t;
typedef struct emdb_pool_s emdb_pool_t;
struct emdb_pool_data_s{
  uint8_t           *last;
  uint8_t           *end;
  emdb_pool_t       *next;
  size_t            failed;
};

struct emdb_pool_s{
  emdb_pool_data_t   d;
  size_t             max;
  emdb_pool_t        *current;
  emdb_pool_large_t  *large;
};

emdb_pool_t* emdb_create_pool(size_t size);
void emdb_destroy_pool(emdb_pool_t *pool);

void* emdb_palloc(emdb_pool_t *pool, size_t size);
void* emdb_pnalloc(emdb_pool_t *pool, size_t size);
void* emdb_pcalloc(emdb_pool_t *pool, size_t size);

#endif
