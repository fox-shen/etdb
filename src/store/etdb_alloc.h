#ifndef H_ETDB_ALLOC_H
#define H_ETDB_ALLOC_H

#if defined(USE_TCMALLOC)
#define malloc(size)       tc_malloc(size)
#define calloc(count,size) tc_calloc(count,size)
#define realloc(ptr,size)  tc_realloc(ptr,size)
#define free(ptr)          tc_free(ptr)
#elif defined(USE_JEMALLOC)
#define malloc(size)       je_malloc(size)
#define calloc(count,size) je_calloc(count,size)
#define realloc(ptr,size)  je_realloc(ptr,size)
#define free(ptr)          je_free(ptr)
#endif

void* etdb_alloc(size_t size);
void* etdb_calloc(size_t size);
void* etdb_realloc(void *old, size_t size);
#define etdb_free(ptr)     free(ptr)
void* etdb_memalign(size_t alignment, size_t size);

#endif
