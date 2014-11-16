#include <emdb.h>
#include <emdb_daemon.h>

/// global variables
pid_t             emdb_pid;
emdb_event_mgr_t  emdb_event_mgr;
emdb_connection_t *emdb_server_conn;
uint8_t           emdb_quit = 0;
uint32_t          emdb_conn_cnt = 0;

static void
emdb_print_welcome()
{
  printf("emdb %s\n", EMDB_VERSION);
  printf("Copyright(c) 2014-2015 tinysky\n");
  printf("\n"); 
}

static void
emdb_print_usage(int argc, char **argv)
{
  printf("Usage:\n");
  printf("     %s [-d] /path/to/emdb.conf\n", argv[0]);
  printf("Options:\n");
  printf("     -d   run as daemon\n");
}

static void
emdb_signal_handler(int sig)
{
  switch(sig){
    case SIGTERM:
    case SIGINT:
      emdb_quit = 1;
      break;

    default:
      break;
  }
}

static void
emdb_init_signal()
{
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT,  emdb_signal_handler);
  signal(SIGTERM, emdb_signal_handler);
}

static void 
emdb_init_server_conn()
{
  emdb_server_conn = emdb_connect_listen("0.0.0.0", 19000);
  if(emdb_server_conn == NULL){
    exit(1);
  }
  emdb_event_mgr_init(&emdb_event_mgr);
  emdb_event_mgr_set(&emdb_event_mgr, EMDB_CONN_FD(emdb_server_conn), FDEVENT_IN, 0, emdb_server_conn);
}

static void
emdb_init(int argc, char **argv)
{
  if(argc < 2){
    emdb_print_usage(argc, argv);
    exit(1);
  }
  emdb_init_server_conn();
  emdb_init_signal();
  if(emdb_module_init() < 0){
    exit(0);
  }
}

static void 
emdb_master_cycle()
{
  const emdb_event_t *events = NULL;
  emdb_queue_t *q;
  emdb_connection_t conn_ready_list_1, conn_ready_list_2;

  while(!emdb_quit)
  {
    emdb_queue_init(&(conn_ready_list_1.queue));
    emdb_queue_init(&(conn_ready_list_2.queue));

    events = emdb_event_mgr_wait(&emdb_event_mgr, 50);
    if(events == NULL)
    {
      break;
    }
    q = emdb_queue_next(&(events->queue));
    for(; q != &(events->queue); q = emdb_queue_next(q))
    {
      emdb_event_t *ev = (emdb_event_t*)q;
      emdb_connection_t *cur_conn = (emdb_connection_t*)ev->data.ptr;

      if(cur_conn == emdb_server_conn)
      { /*** new connection comes ****/
        emdb_connection_t *new_conn = emdb_connect_accept(emdb_server_conn);
        if(new_conn == NULL){
          continue;
        }
        emdb_connect_nodelay(new_conn, 1);
        emdb_connect_noblock(new_conn, 1);
        emdb_event_mgr_set(&emdb_event_mgr, EMDB_CONN_FD(new_conn), FDEVENT_IN, 0, new_conn);
        ++emdb_conn_cnt;
      }
      else if(ev->events & FDEVENT_ERR)
      { /*** error ****/
        --emdb_conn_cnt;
        emdb_event_mgr_del(&emdb_event_mgr, EMDB_CONN_FD(cur_conn));
        emdb_connect_close(cur_conn); 
      }
      else if(ev->events & FDEVENT_IN)
      { /*** read data ****/
        int len = emdb_connect_read(cur_conn); 
        if(len <= 0){
          --emdb_conn_cnt;
          emdb_event_mgr_del(&emdb_event_mgr, EMDB_CONN_FD(cur_conn));
          emdb_connect_close(cur_conn);
        }else{
          emdb_queue_insert_tail(&(conn_ready_list_1.queue), &(cur_conn->queue));       
        }
      }
      else if(ev->events & FDEVENT_OUT)
      { /*** write data ****/
         
      }
    }

    /*** post process ***/ 
    for(q = emdb_queue_next(&(conn_ready_list_1.queue)); q != &(conn_ready_list_1.queue); q = emdb_queue_next(q)){
      emdb_connection_t *conn = (emdb_connection_t*)q;
      emdb_bytes_t *req       = emdb_connect_recv(conn); 
      if(req == NULL){
        --emdb_conn_cnt;
        emdb_event_mgr_del(&emdb_event_mgr, EMDB_CONN_FD(conn));
        emdb_connect_close(conn);
        continue;
      }
      if(emdb_queue_empty(&(req->queue))){
        /*** data not ready ***/
        if(!emdb_event_mgr_isset(&emdb_event_mgr, EMDB_CONN_FD(conn), FDEVENT_IN)){
          emdb_event_mgr_set(&emdb_event_mgr, EMDB_CONN_FD(conn), FDEVENT_IN, 1, conn);  
        }
        continue;
      }
      emdb_command_t* cmd = emdb_module_find_command((emdb_bytes_t*)(req->queue.next));
    }
  }   
}

int
main(int argc, char **argv)
{
  emdb_print_welcome();
  emdb_init(argc, argv);
  emdb_master_cycle();
  return 0;
}
