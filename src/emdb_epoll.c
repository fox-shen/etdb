#ifndef H_EMDB_EPOLL_H
#define H_EMDB_EPOLL_H

void 
emdb_event_mgr_init(emdb_event_mgr_t *event_mgr)
{
  event_mgr->ep_fd      = epoll_create(1024);
  event_mgr->event_size = 8;
  event_mgr->events     = (emdb_event_t**)emdb_alloc(sizeof(emdb_event_t*)*event_mgr->event_size);
  memset(event_mgr->events, 0, sizeof(emdb_event_t*)*event_mgr->event_size);
}

void
emdb_event_mgr_destory(emdb_event_mgr_t *event_mgr)
{
  size_t i = 0;
  for(i = 0; i < event_mgr->event_size; ++i){
     if(event_mgr->events[i] != NULL)
       emdb_free(event_mgr->events[i]);
  }

  if(event_mgr->ep_fd)
    close(event_mgr->ep_fd);
  
  emdb_free(event_mgr->events);
}

int 
emdb_event_mgr_isset(emdb_event_mgr_t *event_mgr, int fd, int flag)
{
  emdb_event_t *ev = emdb_event_mgr_get_event(event_mgr, fd);
  return ev->s_flags & flag;
}

int 
emdb_event_mgr_set(emdb_event_mgr_t *event_mgr, int fd, int flags, int data_num, void *data_ptr)
{
  emdb_event_t *ev = emdb_event_mgr_get_event(event_mgr, fd);
  int ctl_op       = ev->s_flags ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;

  ev->s_flags     |= flags;
  ev->data.num     = data_num;
  ev->data.ptr     = data_ptr;

  struct epoll_event epe;
  epe.data.ptr     = ev;
  epe.events       = 0;
  if(ev->s_flags & FDEVENT_IN)  epe.events |= EPOLLIN;
  if(ev->s_flags & FDEVENT_OUT) epe.events |= EPOLLOUT;

  int ret          = epoll_ctl(event_mgr->ep_fd, ctl_op, fd, &epe);
  if(ret == -1)
    return -1;
  return 0; 
}

int 
emdb_event_mgr_del(emdb_event_mgr_t *event_mgr, int fd)
{
  struct epoll_event epe;
  int ret = epoll_ctl(event_mgr->ep_fd, EPOLL_CTL_DEL, fd, &epe);
  if(ret == -1)
    return -1;
  emdb_event_t *ev = emdb_event_mgr_get_event(event_mgr, fd);
  ev->s_flags      = FDEVENT_NONE;
  return 0;
}

int 
emdb_event_mgr_ctr(emdb_event_mgr_t *event_mgr, int fd, int flags)
{
  emdb_event_t *ev  = emdb_event_mgr_get_event(event_mgr, fd);
  ev->s_flags      &= ~flags;
  int ctl_op        = ev->s_flags ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;

  struct epoll_event epe;
  epe.data.ptr      = ev;
  epe.events        = 0;
  if(ev->s_flags & FDEVENT_IN)   epe.events |= EPOLLIN;
  if(ev->s_flags & FDEVENT_OUT)  epe.events |= EPOLLOUT;
 
  int ret = epoll_ctl(event_mgr->ep_fd, ctl_op, fd, &epe);
  if(ret == -1)
    return -1;
  return 0;
}

const emdb_event_t* 
emdb_event_mgr_wait(emdb_event_mgr_t *event_mgr, int timeout_ms)
{
  emdb_event_t *ev;
  struct epoll_event *epe;
  emdb_queue_init(&(event_mgr->ready_events.queue));

  int i    = 0;
  int nfds = epoll_wait(event_mgr->ep_fd, event_mgr->ep_events, MAX_FDS, timeout_ms);
  if(nfds == -1){
    if(errno == EINTR)
      return &(event_mgr->ready_events);
    return NULL;
  }
  for(i = 0; i < nfds; ++i){
    epe        = &(event_mgr->ep_events[i]);
    ev         = (emdb_event_t*)epe->data.ptr;

    ev->events = FDEVENT_NONE;
    if(epe->events & EPOLLIN)    ev->events |= FDEVENT_IN;
    if(epe->events & EPOLLPRI)   ev->events |= FDEVENT_IN;
    if(epe->events & EPOLLOUT)   ev->events |= FDEVENT_OUT;
    if(epe->events & EPOLLHUP)   ev->events |= FDEVENT_ERR;
    if(epe->events & EPOLLERR)   ev->events |= FDEVENT_ERR;

    emdb_queue_insert_tail(&(event_mgr->ready_events.queue), &(ev->queue));
  }
  return &(event_mgr->ready_events);
}

#endif
