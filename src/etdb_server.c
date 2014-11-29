#include <etdb.h>
#include <etdb_daemon.h>

/// global variables
pid_t             etdb_pid;
etdb_event_mgr_t  etdb_event_mgr;
etdb_connection_t *etdb_server_conn;
uint8_t           etdb_quit = 0;
uint32_t          etdb_conn_cnt = 0;
etdb_log_t        etdb_log;

static void
etdb_print_welcome()
{
  printf("etdb %s\n", ETDB_VERSION);
  printf("Copyright(c) 2014-2015 tinysky\n");
  printf("\n"); 
}

static void
etdb_print_usage(int argc, char **argv)
{
  printf("Usage:\n");
  printf("     %s /path/to/etdb.conf\n", argv[0]);
}

static void
etdb_signal_handler(int sig)
{
  switch(sig){
    case SIGTERM:
    case SIGINT:
      etdb_quit = 1;
      break;
    case SIGCHLD:
      etdb_process_get_status();
      break;

    default:
      break;
  }
}

static void
etdb_init_signal()
{
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT,  etdb_signal_handler);
  signal(SIGTERM, etdb_signal_handler);
  signal(SIGCHLD, etdb_signal_handler);
}

static void 
etdb_init_server_conn()
{
  const char *ip   = etdb_file_config_get_string("IP",  "0.0.0.0");
  int        port  = etdb_file_config_get_int("PORT", 19000);  
  etdb_server_conn = etdb_connect_listen(ip, port);
  if(etdb_server_conn == NULL){
    etdb_log_print(&etdb_log, ETDB_LOG_ERR, "listen on %s:%d failed", ip, port);
  }
  etdb_event_mgr_init(&etdb_event_mgr);
  etdb_event_mgr_set(&etdb_event_mgr, ETDB_CONN_FD(etdb_server_conn), FDEVENT_IN, 0, etdb_server_conn);
}

static void
etdb_init(int argc, char **argv)
{
  if(argc < 2){
    etdb_print_usage(argc, argv);
    exit(1);
  }
  etdb_init_file_config(argv[1]);

  const char *log_file  = etdb_file_config_get_string("LOG_FILE",  "log.txt");
  const char *log_level = etdb_file_config_get_string("LOG_LEVEL", "INFO");
  etdb_log_init(&etdb_log, log_file, log_level);

  if(etdb_file_config_get_int("DAEMON", 0)){
    etdb_daemon();
  }
  etdb_init_server_conn();
  etdb_init_signal();
  etdb_database_init();
  if(etdb_module_init() < 0){
    etdb_log_print(&etdb_log, ETDB_LOG_ERR, "etdb_module_init failed");
  }
}

static void 
etdb_master_cycle()
{
  const etdb_event_t *events = NULL;
  etdb_queue_t *q;
  etdb_connection_t conn_ready_list_1, conn_ready_list_2;

  while(!etdb_quit)
  {
    etdb_queue_init(&(conn_ready_list_1.queue));
    etdb_queue_init(&(conn_ready_list_2.queue));

    events = etdb_event_mgr_wait(&etdb_event_mgr, 50);
    if(events == NULL)
    {
      break;
    }
    q = etdb_queue_next(&(events->queue));
    for(; q != &(events->queue); q = etdb_queue_next(q))
    {
      etdb_event_t *ev = (etdb_event_t*)q;
      etdb_connection_t *cur_conn = (etdb_connection_t*)ev->data.ptr;

      if(cur_conn == etdb_server_conn)
      { /*** new connection comes ****/
        etdb_connection_t *new_conn = etdb_connect_accept(etdb_server_conn);
        if(new_conn == NULL){
          continue;
        }
        etdb_connect_nodelay(new_conn, 1);
        etdb_connect_noblock(new_conn, 1);
        etdb_event_mgr_set(&etdb_event_mgr, ETDB_CONN_FD(new_conn), FDEVENT_IN, 0, new_conn);
        ++etdb_conn_cnt;
      }
      else if(ev->events & FDEVENT_ERR)
      { /*** error ****/
        --etdb_conn_cnt;
        etdb_event_mgr_del(&etdb_event_mgr, ETDB_CONN_FD(cur_conn));
        etdb_connect_close(cur_conn); 
      }
      else if(ev->events & FDEVENT_IN)
      { /*** read data ****/
        int len = etdb_connect_read_to_buf(cur_conn); 
        if(len <= 0){
          --etdb_conn_cnt;
          etdb_event_mgr_del(&etdb_event_mgr, ETDB_CONN_FD(cur_conn));
          etdb_connect_close(cur_conn);
        }else{
          etdb_queue_insert_tail(&(conn_ready_list_1.queue), &(cur_conn->queue));       
        }
      }
      else if(ev->events & FDEVENT_OUT)
      { /*** write data ****/
        int len = etdb_connect_write(cur_conn);
        if(len <= 0){
          --etdb_conn_cnt;
          etdb_event_mgr_del(&etdb_event_mgr, ETDB_CONN_FD(cur_conn));
          etdb_connect_close(cur_conn);
        }else if(cur_conn->buf_out->size == 0){
          etdb_event_mgr_ctr(&etdb_event_mgr, ETDB_CONN_FD(cur_conn), FDEVENT_OUT);
          if(cur_conn->buf_in->size != 0){
            etdb_queue_insert_tail(&(conn_ready_list_1.queue), &(cur_conn->queue));
          }else{
            etdb_event_mgr_set(&etdb_event_mgr, ETDB_CONN_FD(cur_conn), FDEVENT_IN, 1, cur_conn);
          }
        }
      }
    }

    /*** post process ***/ 
    for(q = etdb_queue_next(&(conn_ready_list_1.queue)); q != &(conn_ready_list_1.queue); q = etdb_queue_next(q)){
      etdb_connection_t *conn = (etdb_connection_t*)q;
      etdb_bytes_t *req       = etdb_connect_recv(conn); 
      if(req == NULL){
        --etdb_conn_cnt;
        etdb_event_mgr_del(&etdb_event_mgr, ETDB_CONN_FD(conn));
        etdb_connect_close(conn);
        continue;
      }
      if(etdb_queue_empty(&(req->queue))){
        /*** data not ready ***/
        if(!etdb_event_mgr_isset(&etdb_event_mgr, ETDB_CONN_FD(conn), FDEVENT_IN)){
          etdb_event_mgr_set(&etdb_event_mgr, ETDB_CONN_FD(conn), FDEVENT_IN, 1, conn);  
        }
        continue;
      }      
      etdb_serv_exec_proc(req, conn);
    }
  }   
}

void
etdb_server_exit()
{
   
}

int
main(int argc, char **argv)
{
  etdb_print_welcome();
  etdb_init(argc, argv);
  etdb_master_cycle();
  etdb_server_exit();
  return 0;
}
