#include <etdb.h>

etdb_event_mgr_t  etdb_event_mgr;
etdb_connection_t *etdb_cli_conn;
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
    for(; i < etdb_parallel_conn; ++i){
      etdb_cli_conn = etdb_connect_request(etdb_serv_ip, etdb_serv_port);
      if(etdb_cli_conn == NULL){
         fprintf(stderr, "connect to %s:%d failed\n", etdb_serv_ip, etdb_serv_port);
         exit(1);
      }
      etdb_event_mgr_set(&etdb_event_mgr, ETDB_CONN_FD(etdb_cli_conn), FDEVENT_IN, 0, etdb_cli_conn);      
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

static void
etdb_cli_cycle()
{
  const etdb_event_t *events = NULL;
  etdb_queue_t *q;

  while(!etdb_quit)
  {
    events = etdb_event_mgr_wait(&etdb_event_mgr, 50);
    if(events == NULL)
    {
      break;
    }    

    q = etdb_queue_next(&(events->queue));
    for(; q != &(events->queue); q = etdb_queue_next(q))
    {
       fprintf(stderr, "ssss\n");
    }

    
  }
}

static void
etdb_exec_benchmark(const char *cmd)
{
  
}

int 
main(int argc, char **argv)
{
  etdb_cli_welcome();
  etdb_cli_init(argc, argv); 
  if(!etdb_benchmark){
     etdb_cli_cycle();    
  }else{
     etdb_exec_benchmark("set");
  }
  return 0;
}
