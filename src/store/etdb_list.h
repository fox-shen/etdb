#ifndef H_ETDB_LIST_H
#define H_ETDB_LIST_H

typedef struct etdb_list_s etdb_list_t;
struct etdb_list_s{
  uint32_t       size;
  etdb_queue_t   queue;
  uint8_t        data[0]; 
};

#define etdb_list_next(list)  \
((etdb_list_t*)((uint8_t*)((list)->queue.next) - offsetof(etdb_list_t, queue)))

#define etdb_list_prev(list)  \
((etdb_list_t*)((uint8_t*)((list)->queue.prev) - offsetof(etdb_list_t, queue)))

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

void
etdb_list_lpop(etdb_list_t *head)
{
  etdb_list_t *l = etdb_list_next(head); 
  etdb_queue_remove(&(l->queue));
  etdb_free(l);
}

void
etdb_list_rpop(etdb_list_t *head)
{
  etdb_list_t *l = etdb_list_prev(head);
  etdb_queue_remove(&(l->queue));
  etdb_free(l);
}

#endif
