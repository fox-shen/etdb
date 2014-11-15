#include <emdb.h>

static emdb_event_t* 
emdb_event_mgr_get_event(emdb_event_mgr_t *mgr, int fd)
{
   if(mgr->event_size <= fd){
      mgr->events                  = (emdb_event_t**)emdb_realloc(mgr->events, sizeof(emdb_event_t*)*(fd + 1));
      memset(mgr->events + mgr->event_size, 0, sizeof(emdb_event_t*)*(fd + 1 - mgr->event_size));
      mgr->event_size              = fd + 1;   
   }
   if(mgr->events[fd] == NULL){
      mgr->events[fd]               = (emdb_event_t*)emdb_alloc(sizeof(emdb_event_t));
      mgr->events[fd]->fd           = fd;
      mgr->events[fd]->s_flags      = FDEVENT_NONE;
      mgr->events[fd]->data.num     = 0;
      mgr->events[fd]->data.ptr     = NULL;
   }  
   return mgr->events[fd];
}

#include <emdb_epoll.c>
