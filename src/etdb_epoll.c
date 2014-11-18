#ifndef H_ETDB_EPOLL_H
#define H_ETDB_EPOLL_H

void 
etdb_event_mgr_init(etdb_event_mgr_t *event_mgr)
{
  event_mgr->ep_fd      = epoll_create(1024);
  event_mgr->event_size = 8;
  event_mgr->events     = (etdb_event_t**)etdb_alloc(sizeof(etdb_event_t*)*event_mgr->event_size);
  memset(event_mgr->events, 0, sizeof(etdb_event_t*)*event_mgr->event_size);
}

void
etdb_event_mgr_destory(etdb_event_mgr_t *event_mgr)
{
  size_t i = 0;
  for(i = 0; i < event_mgr->event_size; ++i){
     if(event_mgr->events[i] != NULL)
       etdb_free(event_mgr->events[i]);
  }

  if(event_mgr->ep_fd)
    close(event_mgr->ep_fd);
  
  etdb_free(event_mgr->events);
}

int 
etdb_event_mgr_isset(etdb_event_mgr_t *event_mgr, int fd, int flag)
{
  etdb_event_t *ev = etdb_event_mgr_get_event(event_mgr, fd);
  return ev->s_flags & flag;
}

int 
etdb_event_mgr_set(etdb_event_mgr_t *event_mgr, int fd, int flags, int data_num, void *data_ptr)
{
  etdb_event_t *ev = etdb_event_mgr_get_event(event_mgr, fd);
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
etdb_event_mgr_del(etdb_event_mgr_t *event_mgr, int fd)
{
  struct epoll_event epe;
  int ret = epoll_ctl(event_mgr->ep_fd, EPOLL_CTL_DEL, fd, &epe);
  if(ret == -1)
    return -1;
  etdb_event_t *ev = etdb_event_mgr_get_event(event_mgr, fd);
  ev->s_flags      = FDEVENT_NONE;
  return 0;
}

int 
etdb_event_mgr_ctr(etdb_event_mgr_t *event_mgr, int fd, int flags)
{
  etdb_event_t *ev  = etdb_event_mgr_get_event(event_mgr, fd);
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

const etdb_event_t* 
etdb_event_mgr_wait(etdb_event_mgr_t *event_mgr, int timeout_ms)
{
  etdb_event_t *ev;
  struct epoll_event *epe;
  etdb_queue_init(&(event_mgr->ready_events.queue));

  int i    = 0;
  int nfds = epoll_wait(event_mgr->ep_fd, event_mgr->ep_events, MAX_FDS, timeout_ms);
  if(nfds == -1){
    if(errno == EINTR)
      return &(event_mgr->ready_events);
    return NULL;
  }
  for(i = 0; i < nfds; ++i){
    epe        = &(event_mgr->ep_events[i]);
    ev         = (etdb_event_t*)epe->data.ptr;

    ev->events = FDEVENT_NONE;
    if(epe->events & EPOLLIN)    ev->events |= FDEVENT_IN;
    if(epe->events & EPOLLPRI)   ev->events |= FDEVENT_IN;
    if(epe->events & EPOLLOUT)   ev->events |= FDEVENT_OUT;
    if(epe->events & EPOLLHUP)   ev->events |= FDEVENT_ERR;
    if(epe->events & EPOLLERR)   ev->events |= FDEVENT_ERR;

    etdb_queue_insert_tail(&(event_mgr->ready_events.queue), &(ev->queue));
  }
  return &(event_mgr->ready_events);
}

#endif
