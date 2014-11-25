#ifndef H_ETDB_STACK_H
#define H_ETDB_STACK_H

typedef struct etdb_stack_s etdb_stack_t;
struct etdb_stack_s{
  etdb_pool_t *pool;
  size_t       nalloc;
  size_t       nelts;
  size_t       size;
  void        *elts;
};

void  etdb_stack_init(etdb_stack_t *stack, etdb_pool_t *pool, size_t n, size_t size);
void* etdb_stack_push(etdb_stack_t *stack);
void* etdb_stack_pop(etdb_stack_t *stack);
void* etdb_stack_top(etdb_stack_t *stack);

#define etdb_stack_empty(stack)    ((stack)->nelts == 0)
#define etdb_stack_size(stack)     (stack)->nelts

#endif
