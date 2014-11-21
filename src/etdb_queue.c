#include <etdb.h>

int 
etdb_queue_count(etdb_queue_t *q)
{
  etdb_queue_t *n = q->next;
  int cnt = 0;
  for(; n != q; n = n->next)  ++cnt;
  return cnt;
}
