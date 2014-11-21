#include <etdb.h>

etdb_event_mgr_t  etdb_event_mgr;
etdb_connection_t *etdb_cli_conn;
etdb_connection_t **etdb_cli_conn_tab;
uint8_t           etdb_quit  = 0;
char etdb_serv_ip[255]       = "127.0.0.1";
short int etdb_serv_port     = 8218;
short int etdb_benchmark     = 0;
int       etdb_request_num   = 10000;
int       etdb_parallel_conn = 50; 

static void 
etdb_cli_welcome()
{
  printf("etdb cli, %s\n", ETDB_VERSION);
  printf("Copyright (c) 2014-2015 tinysky\n");
  printf("\n");
}

static void 
etdb_cli_usage(int argc, char **argv)
{
  printf("Usage:\n");
  printf("    %s ip port [-b -r{num} -c{num}]\n", argv[0]);
  printf("\n");
  printf("Options:\n");
  printf("    ip          server ip (default 127.0.0.1)\n");
  printf("    port        server port (default 8218)\n");
  printf("    -b          do benchmark\n");
  printf("    -r          Total number of requests (default 10000)\n");
  printf("    -c          Number of parallel connections (default 50)\n");
  printf("\n");
}

static void
etdb_cli_init_conn()
{
  etdb_event_mgr_init(&etdb_event_mgr);

  if(!etdb_benchmark){
    etdb_cli_conn = etdb_connect_request(etdb_serv_ip, etdb_serv_port);
    if(etdb_cli_conn == NULL){
      fprintf(stderr, "connect to %s:%d failed\n", etdb_serv_ip, etdb_serv_port);
      exit(1);
    }
    etdb_event_mgr_set(&etdb_event_mgr, ETDB_CONN_FD(etdb_cli_conn), FDEVENT_IN, 0, etdb_cli_conn);
  }else{
    int i = 0;
    etdb_cli_conn_tab = etdb_alloc(sizeof(etdb_connection_t*)*etdb_parallel_conn);
    for(; i < etdb_parallel_conn; ++i){
      etdb_cli_conn = etdb_connect_request(etdb_serv_ip, etdb_serv_port);
      if(etdb_cli_conn == NULL){
         fprintf(stderr, "connect to %s:%d failed\n", etdb_serv_ip, etdb_serv_port);
         exit(1);
      }
      etdb_event_mgr_set(&etdb_event_mgr, ETDB_CONN_FD(etdb_cli_conn), FDEVENT_IN, 0, etdb_cli_conn);   
      etdb_cli_conn_tab[i] = etdb_cli_conn;
    } 
  }
}

static void
etdb_cli_init(int argc, char **argv)
{
  if(argc < 3){
    etdb_cli_usage(argc, argv);
    exit(1);
  }
  sprintf(etdb_serv_ip, "%s", argv[1]);
  etdb_serv_port = atoi(argv[2]);

  int pos = 2;
  for(; pos < argc; ++pos){
    if( strncmp(argv[pos], "-b", 2) == 0 ){
      etdb_benchmark = 1;
    }else if( strncmp(argv[pos], "-r", 2) == 0 ){
      int tmp = atoi(argv[pos] + 2);
      if(tmp > 0)  etdb_request_num = tmp;
    }else if( strncmp(argv[pos], "-c", 2) == 0 ){
      int tmp = atoi(argv[pos] + 2);
      if(tmp > 0)  etdb_parallel_conn = tmp;
    }
  }
  
  etdb_cli_init_conn();
}

static int
etdb_cli_handle_user_cmd(etdb_connection_t *cur_conn, uint8_t *cmd, size_t len)
{
  if( etdb_connect_send_cmd(cur_conn, cmd, len) < 0 )
    return -1; 
  size_t size = cur_conn->buf_out->size;
  if(etdb_connect_write(cur_conn) != size){
    if(!etdb_event_mgr_isset(&etdb_event_mgr, ETDB_CONN_FD(cur_conn), FDEVENT_OUT)){
      etdb_event_mgr_set(&etdb_event_mgr, ETDB_CONN_FD(cur_conn), FDEVENT_OUT, 1, cur_conn);
    }
  }
  return 0;
}

static void
etdb_cli_cycle()
{
  const etdb_event_t *events = NULL;
  etdb_queue_t *q;
  short int responsed = 1;
  char *buf;
  etdb_connection_t conn_ready_list;
  char title[256];
  sprintf(title, "%s:%d>", etdb_serv_ip, etdb_serv_port);

  while(!etdb_quit)
  {
    if(responsed == 1)
    {
      buf = linenoise(title);
      if(buf == NULL)   break; 

      if(strcmp(buf, "quit") == 0){
        printf("Bye\n");  exit(0);
      }

      if( etdb_cli_handle_user_cmd(etdb_cli_conn, buf, strlen(buf)) < 0 )  continue;
      responsed = 0;
    }

    etdb_queue_init(&(conn_ready_list.queue));
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

      if(ev->events & FDEVENT_ERR)
      { /*** error ****/
        etdb_event_mgr_del(&etdb_event_mgr, ETDB_CONN_FD(cur_conn));
        etdb_connect_close(cur_conn);
        etdb_quit = 1;
      }
      else if(ev->events & FDEVENT_IN)
      { /*** read data ***/
        int len = etdb_connect_read_to_buf(cur_conn);
        if(len <= 0){
          etdb_event_mgr_del(&etdb_event_mgr, ETDB_CONN_FD(cur_conn));
          etdb_connect_close(cur_conn);
          etdb_quit = 1;
        }else{
          etdb_queue_insert_tail(&(conn_ready_list.queue), &(cur_conn->queue));
        }
      }
      else if(ev->events & FDEVENT_OUT)
      {
        /*** write data ****/
        int len = etdb_connect_write(cur_conn);
        if(len <= 0){
          etdb_event_mgr_del(&etdb_event_mgr, ETDB_CONN_FD(cur_conn));
          etdb_connect_close(cur_conn);
          etdb_quit = 1;
        }else if(cur_conn->buf_out->size == 0){
          etdb_event_mgr_ctr(&etdb_event_mgr, ETDB_CONN_FD(cur_conn), FDEVENT_OUT);
          if(cur_conn->buf_in->size != 0){
            etdb_queue_insert_tail(&(conn_ready_list.queue), &(cur_conn->queue));
          }else{
            etdb_event_mgr_set(&etdb_event_mgr, ETDB_CONN_FD(cur_conn), FDEVENT_IN, 1, cur_conn);
          }
        } 
      } 
    }

    /*** post process ***/  
    for(q = etdb_queue_next(&(conn_ready_list.queue)); q != &(conn_ready_list.queue); q = etdb_queue_next(q)){
      etdb_connection_t *conn = (etdb_connection_t*)q;
      etdb_bytes_t *req       = etdb_connect_recv(conn);
      if(req == NULL){
        etdb_event_mgr_del(&etdb_event_mgr, ETDB_CONN_FD(conn));
        etdb_connect_close(conn);
        etdb_quit = 1;
        continue;
      }
      if(etdb_queue_empty(&(req->queue))){
        /*** data not ready ***/
        if(!etdb_event_mgr_isset(&etdb_event_mgr, ETDB_CONN_FD(conn), FDEVENT_IN)){
          etdb_event_mgr_set(&etdb_event_mgr, ETDB_CONN_FD(conn), FDEVENT_IN, 1, conn);
        }
        continue;
      }
      
      etdb_queue_t *qq = req->queue.next;
      int resp_cnt     = etdb_queue_count(&(req->queue));
      etdb_bytes_t *bb = (etdb_bytes_t*)qq;
      if(resp_cnt > 1)
        bb = (etdb_bytes_t*)(qq->next);

      char data[1024];
      memset(data, 0, sizeof(data));
      memcpy(data, bb->str.data, bb->str.len);  
      printf("%s\n", data);
      fflush(stdout);
      responsed = 1;
    }
  }
}

static void
etdb_exec_benchmark_cycle(uint8_t** requests_data)
{
  const etdb_event_t *events = NULL;
  etdb_queue_t *q;
  etdb_connection_t conn_ready_list;
  long int time_start = etdb_utls_get_timestamp();
  int num_sent;

  for(num_sent = 0; num_sent < etdb_parallel_conn; ++num_sent){
    etdb_cli_handle_user_cmd(etdb_cli_conn_tab[num_sent], requests_data[num_sent], strlen(requests_data[num_sent]));
  }

  while(!etdb_quit)
  {
    etdb_queue_init(&(conn_ready_list.queue));
    events = etdb_event_mgr_wait(&etdb_event_mgr, 50);
    if(events == NULL)  break;

    q = etdb_queue_next(&(events->queue));
    for(; q != &(events->queue); q = etdb_queue_next(q))
    {
      etdb_event_t *ev = (etdb_event_t*)q;
      etdb_connection_t *cur_conn = (etdb_connection_t*)ev->data.ptr;

      if(ev->events & FDEVENT_ERR)
      { /*** error ****/
        etdb_event_mgr_del(&etdb_event_mgr, ETDB_CONN_FD(cur_conn));
        etdb_connect_close(cur_conn);
        etdb_quit = 1;
      }
      else if(ev->events & FDEVENT_IN)
      { /*** read data ***/
        int len = etdb_connect_read_to_buf(cur_conn);
        if(len <= 0){
          etdb_event_mgr_del(&etdb_event_mgr, ETDB_CONN_FD(cur_conn));
          etdb_connect_close(cur_conn);
          etdb_quit = 1;
        }else{
          etdb_queue_insert_tail(&(conn_ready_list.queue), &(cur_conn->queue));
        }
      }
      else if(ev->events & FDEVENT_OUT)
      {
        /*** write data ****/
        int len = etdb_connect_write(cur_conn);
        if(len <= 0){
          etdb_event_mgr_del(&etdb_event_mgr, ETDB_CONN_FD(cur_conn));
          etdb_connect_close(cur_conn);
          etdb_quit = 1;
        }else if(cur_conn->buf_out->size == 0){
          etdb_event_mgr_ctr(&etdb_event_mgr, ETDB_CONN_FD(cur_conn), FDEVENT_OUT);
          if(cur_conn->buf_in->size != 0){
            etdb_queue_insert_tail(&(conn_ready_list.queue), &(cur_conn->queue));
          }else{
            etdb_event_mgr_set(&etdb_event_mgr, ETDB_CONN_FD(cur_conn), FDEVENT_IN, 1, cur_conn);
          }
        }
      }
    }

    /*** process ready list ***/ 
    for(q = etdb_queue_next(&(conn_ready_list.queue)); q != &(conn_ready_list.queue); q = etdb_queue_next(q)){
      etdb_connection_t *conn = (etdb_connection_t*)q;
      etdb_bytes_t *req       = etdb_connect_recv(conn);
      if(req == NULL){
        etdb_event_mgr_del(&etdb_event_mgr, ETDB_CONN_FD(conn));
        etdb_connect_close(conn);
        etdb_quit = 1;
        continue;
      }
      if(etdb_queue_empty(&(req->queue))){
        /*** data not ready ***/
        if(!etdb_event_mgr_isset(&etdb_event_mgr, ETDB_CONN_FD(conn), FDEVENT_IN)){
          etdb_event_mgr_set(&etdb_event_mgr, ETDB_CONN_FD(conn), FDEVENT_IN, 1, conn);
        }
        continue;
      }
     
      etdb_queue_t *qq = req->queue.next;
      if(memcmp(((etdb_bytes_t*)qq)->str.data, "+OK", sizeof("+OK") - 1) == 0){
        if(++num_sent < etdb_request_num){
           etdb_cli_handle_user_cmd(conn, requests_data[num_sent], strlen(requests_data[num_sent])); 
        }else{
          /*** output qps***/
          long int time_end = etdb_utls_get_timestamp();
          fprintf(stdout, "request: %d, time: %d ms \n", 
                           etdb_request_num,
                           ((time_end - time_start)/1000));
          return;
        }    
      }else{
        char err_str[1024];
        memset(err_str, 0, sizeof(err_str));
        memcpy(err_str, ((etdb_bytes_t*)qq)->str.data, ((etdb_bytes_t*)qq)->str.len);
        fprintf(stderr, "bad resp: %s\n", err_str);
        exit(0);
      }
    }
  }
}

static void
etdb_exec_benchmark_kv()
{
  int num = 0;
  uint8_t** requests_data = etdb_alloc(sizeof(char*)*etdb_request_num);
  for(; num < etdb_request_num; ++num){
    uint8_t buf[1024], pack_cmd[2048];
    int n = rand();
    snprintf(buf, sizeof(buf), "k%010d", n);
    sprintf(pack_cmd, "set %s 1", buf);
    requests_data[num] = etdb_alloc(sizeof(uint8_t)*strlen(pack_cmd) + 1);
    requests_data[num][strlen(pack_cmd)] = '\0';
    memcpy(requests_data[num], pack_cmd, strlen(pack_cmd));
  }
  fprintf(stdout, "Start Set Benchmark\n"); 
  etdb_exec_benchmark_cycle(requests_data);

  for(num = 0; num < etdb_request_num; ++num){
    memcpy(requests_data[num], "get", 3);
    requests_data[num][strlen(requests_data[num]) - 2] = '\0';
  }
  fprintf(stdout, "Start Get Benchmark\n");
  etdb_exec_benchmark_cycle(requests_data); 
}

int 
main(int argc, char **argv)
{
  etdb_cli_welcome();
  etdb_cli_init(argc, argv); 
  if(!etdb_benchmark){
     etdb_cli_cycle();    
  }else{
     etdb_exec_benchmark_kv();
  }
  return 0;
}
