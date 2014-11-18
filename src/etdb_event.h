#ifndef H_ETDB_EVENT_H
#define H_ETDB_EVENT_H

#define FDEVENT_NONE       (0)
#define FDEVENT_IN         (1<<0)
#define FDEVENT_PRI        (1<<1)
#define FDEVENT_OUT        (1<<2)
#define FDEVENT_HUP        (1<<3)
#define FDEVENT_ERR        (1<<4)

typedef struct etdb_event_s etdb_event_t;
struct etdb_event_s{
  etdb_queue_t        queue;
  int                 fd;
  int                 s_flags;
  int                 events;
  struct{
    int               num;
    void              *ptr;
  }data;
};

#define MAX_FDS       8*1024

typedef struct etdb_event_mgr_s etdb_event_mgr_t;
struct etdb_event_mgr_s{
  etdb_event_t        **events;
  size_t              event_size;
  etdb_event_t        ready_events;
  int                 ep_fd;
  struct epoll_event  ep_events[MAX_FDS]; 
};

void
etdb_event_mgr_init(etdb_event_mgr_t *event_mgr);
void 
etdb_event_mgr_destory(etdb_event_mgr_t *event_mgr);
int 
etdb_event_mgr_isset(etdb_event_mgr_t *event_mgr, int fd, int flag);
int 
etdb_event_mgr_set(etdb_event_mgr_t *event_mgr, int fd, int flags, int data_num, void *data_ptr);
int 
etdb_event_mgr_del(etdb_event_mgr_t *event_mgr, int fd);
int 
etdb_event_mgr_ctr(etdb_event_mgr_t *event_mgr, int fd, int flags);
const etdb_event_t* 
etdb_event_mgr_wait(etdb_event_mgr_t *event_mgr, int timeout_ms);

#endif
