#ifndef H_ETDB_QUEUE_H
#define H_ETDB_QUEUE_H

typedef struct etdb_queue_s etdb_queue_t;
struct etdb_queue_s{
   etdb_queue_t *prev;
   etdb_queue_t *next;
};

#define etdb_queue_init(q)                                                     \
    (q)->prev = q;                                                             \
    (q)->next = q

#define etdb_queue_empty(h)                                                    \
    (h == (h)->prev)

#define etdb_queue_insert_head(h, x)                                           \
    (x)->next = (h)->next;                                                     \
    (x)->next->prev = x;                                                       \
    (x)->prev = h;                                                             \
    (h)->next = x

#define etdb_queue_insert_after   etdb_queue_insert_head

#define etdb_queue_insert_tail(h, x)                                           \
    (x)->prev = (h)->prev;                                                     \
    (x)->prev->next = x;                                                       \
    (x)->next = h;                                                             \
    (h)->prev = x

#define etdb_queue_head(h)                                                     \
    (h)->next

#define etdb_queue_last(h)                                                     \
    (h)->prev

#define etdb_queue_sentinel(h)                                                 \
    (h)

#define etdb_queue_next(q)                                                     \
    (q)->next

#define etdb_queue_prev(q)                                                     \
    (q)->prev

#define etdb_queue_remove(x)                                                   \
    (x)->next->prev = (x)->prev;                                               \
    (x)->prev->next = (x)->next   

#define etdb_queue_add(h, n)                                                   \
    (h)->prev->next = (n)->next;                                               \
    (n)->next->prev = (h)->prev;                                               \
    (h)->prev = (n)->prev;                                                     \
    (h)->prev->next = h;

#endif
