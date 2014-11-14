#ifndef H_EMDB_ALLOC_H
#define H_EMDB_ALLOC_H

void* emdb_alloc(size_t size);
void* emdb_calloc(size_t size);
void* emdb_realloc(void *old, size_t size);
#define emdb_free free
void* emdb_memalign(size_t alignment, size_t size);

#endif
