#include <etdb.h>

void 
etdb_stack_init(etdb_stack_t *stack, etdb_pool_t *pool, size_t n, size_t size)
{
  stack->pool   = pool;
  stack->nalloc = n;
  stack->size   = size;
  stack->nelts  = 0;
  stack->elts   = etdb_palloc(stack->pool, stack->nalloc*size); 
}

void* 
etdb_stack_push(etdb_stack_t *stack)
{
  etdb_pool_t *p;
  size_t size;
  void *elts;

  if(stack->nelts == stack->nalloc){
     size = stack->size * stack->nalloc;
     p    = stack->pool;

     if((uint8_t*)stack->elts + size == p->d.last
            && p->d.last + stack->size <= p->d.end)
     {
       p->d.last += stack->size;
       ++(stack->nalloc);
     }
     else
     {
        void *new = etdb_palloc(p, 2 * size);
        if (new == NULL) {
          return NULL;
        }
        memcpy(new, stack->elts, size);
        stack->elts    = new;
        stack->nalloc *= 2;       
     } 
  }
  elts = (uint8_t*)stack->elts + stack->nelts * stack->size;
  ++(stack->nelts);
  return elts;
}

void* 
etdb_stack_pop(etdb_stack_t *stack)
{
  --(stack->nelts);
  return (uint8_t*)stack->elts + stack->nelts * stack->size;
}

void* 
etdb_stack_top(etdb_stack_t *stack)
{
  return (uint8_t*)stack->elts + (stack->nelts - 1) * stack->size;
}


