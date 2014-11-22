#include <etdb.h>

int 
etdb_queue_count(etdb_queue_t *q)
{
  etdb_queue_t *n = q->next;
  int cnt = 0;
  for(; n != q; n = n->next)  ++cnt;
  return cnt;
}

void 
etdb_queue_free(etdb_queue_t *h)
{
  etdb_queue_t *p = h;
  etdb_queue_t *n = p->next;  
  etdb_free(p);

  for(; n != h; ){ 
    p = n;
    n = n->next;
    etdb_free(p);
  }
}
