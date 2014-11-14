#ifndef H_EMDB_QUEUE_H
#define H_EMDB_QUEUE_H

typedef struct emdb_queue_s emdb_queue_t;
struct emdb_queue_s{
   emdb_queue_t *prev;
   emdb_queue_t *next;
};

#define emdb_queue_init(q)                                                     \
    (q)->prev = q;                                                             \
    (q)->next = q

#define emdb_queue_empty(h)                                                    \
    (h == (h)->prev)

#define emdb_queue_insert_head(h, x)                                           \
    (x)->next = (h)->next;                                                     \
    (x)->next->prev = x;                                                       \
    (x)->prev = h;                                                             \
    (h)->next = x

#define emdb_queue_insert_after   emdb_queue_insert_head

#define emdb_queue_insert_tail(h, x)                                           \
    (x)->prev = (h)->prev;                                                     \
    (x)->prev->next = x;                                                       \
    (x)->next = h;                                                             \
    (h)->prev = x

#define emdb_queue_head(h)                                                     \
    (h)->next

#define emdb_queue_last(h)                                                     \
    (h)->prev

#define emdb_queue_sentinel(h)                                                 \
    (h)

#define emdb_queue_next(q)                                                     \
    (q)->next

#define emdb_queue_prev(q)                                                     \
    (q)->prev

#define emdb_queue_remove(x)                                                   \
    (x)->next->prev = (x)->prev;                                               \
    (x)->prev->next = (x)->next   

#define emdb_queue_add(h, n)                                                   \
    (h)->prev->next = (n)->next;                                               \
    (n)->next->prev = (h)->prev;                                               \
    (h)->prev = (n)->prev;                                                     \
    (h)->prev->next = h;

#endif
