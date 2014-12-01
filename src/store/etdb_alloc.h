#ifndef H_ETDB_ALLOC_H
#define H_ETDB_ALLOC_H

void* etdb_alloc(size_t size);
void* etdb_calloc(size_t size);
void* etdb_realloc(void *old, size_t size);
void  etdb_free(void *old);
void* etdb_memalign(size_t alignment, size_t size);

#endif
