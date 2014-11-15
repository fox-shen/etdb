#ifndef H_EMDB_EVENT_H
#define H_EMDB_EVENT_H

#define FDEVENT_NONE       (0)
#define FDEVENT_IN         (1<<0)
#define FDEVENT_PRI        (1<<1)
#define FDEVENT_OUT        (1<<2)
#define FDEVENT_HUP        (1<<3)
#define FDEVENT_ERR        (1<<4)

typedef struct emdb_event_s emdb_event_t;
struct emdb_event_s{
  emdb_queue_t        queue;
  int                 fd;
  int                 s_flags;
  int                 events;
  struct{
    int               num;
    void              *ptr;
  }data;
};

#define MAX_FDS       8*1024

typedef struct emdb_event_mgr_s emdb_event_mgr_t;
struct emdb_event_mgr_s{
  emdb_event_t        **events;
  size_t              event_size;
  emdb_event_t        ready_events;
  int                 ep_fd;
  struct epoll_event  ep_events[MAX_FDS]; 
};

void
emdb_event_mgr_init(emdb_event_mgr_t *event_mgr);
void 
emdb_event_mgr_destory(emdb_event_mgr_t *event_mgr);
int 
emdb_event_mgr_isset(emdb_event_mgr_t *event_mgr, int fd, int flag);
int 
emdb_event_mgr_set(emdb_event_mgr_t *event_mgr, int fd, int flags, int data_num, void *data_ptr);
int 
emdb_event_mgr_del(emdb_event_mgr_t *event_mgr, int fd);
int 
emdb_event_mgr_ctr(emdb_event_mgr_t *event_mgr, int fd, int flags);
const emdb_event_t* 
emdb_event_mgr_wait(emdb_event_mgr_t *event_mgr, int timeout_ms);

#endif
