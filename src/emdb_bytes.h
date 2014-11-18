#ifndef H_EMDB_BYTES_H
#define H_EMDB_BYTES_H

typedef struct emdb_bytes_s{
  emdb_queue_t    queue;
  uint8_t         *data;
  int             size;
}emdb_bytes_t;

#define emdb_bytes_set(emdb_bytes, ptr, len) {(emdb_bytes)->data = ptr; (emdb_bytes)->size = len;}
#define emdb_bytes_compare(b1, b2, r)                                        \
{                                                                            \
   const int min_len = b1->size < b2->size ? b1->size : b2->size;            \
   r = memcmp(b1->data, b2->data, min_len);                                  \
   if(r == 0){                                                               \
      if(b1->size < b2->size) r = -1;                                        \
      else if(b1->size > b2->size) r = +1;                                   \
   }                                                                         \
}

#endif
