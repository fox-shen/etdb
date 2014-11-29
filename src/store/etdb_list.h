#ifndef H_ETDB_LIST_H
#define H_ETDB_LIST_H

typedef struct etdb_list_s etdb_list_t;
struct etdb_list_s{
  etdb_queue_t   queue;
  size_t         size;
  uint8_t        data[0]; 
};

etdb_list_t*
etdb_list_new()
{
  etdb_list_t *new_list = (etdb_list_t*)etdb_alloc(sizeof(etdb_list_t));
  new_list->size        =  0;
  etdb_queue_init(&(new_list->queue));
  return new_list;
}

int
etdb_list_lpush(etdb_list_t *head, const uint8_t *value, size_t len)
{
  etdb_list_t *new_list = (etdb_list_t*)etdb_alloc(sizeof(etdb_list_t) + len);
  new_list->size        = len;
  memcpy(new_list->data, value, len);
  etdb_queue_insert_head(&(head->queue), &(new_list->queue)); 
  return 0;
}

int
etdb_list_rpush(etdb_list_t *head, const uint8_t *value, size_t len)
{
  etdb_list_t *new_list = (etdb_list_t*)etdb_alloc(sizeof(etdb_list_t) + len);
  new_list->size        = len; 
  memcpy(new_list->data, value, len);

  etdb_queue_insert_tail(&(head->queue), &(new_list->queue));
  return 0;
}

int
etdb_list_empty(etdb_list_t *head)
{
  return etdb_queue_empty(&(head->queue));
}

int
etdb_list_size(etdb_list_t *head)
{
  int size = 0;
  etdb_queue_t *n = head->queue.next;
  for(; n != &(head->queue); n = n->next)  ++size;
  return size;
}

etdb_list_t*
etdb_list_lpop(etdb_list_t *head)
{
  etdb_list_t *l = (etdb_list_t*)head->queue.next; 
  etdb_queue_remove(&(l->queue));
  return l;
}

etdb_list_t*
etdb_list_rpop(etdb_list_t *head)
{
  etdb_list_t *l = (etdb_list_t*)head->queue.prev;
  etdb_queue_remove(&(l->queue));
  return l;
}

#endif
