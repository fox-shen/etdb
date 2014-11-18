#include <emdb.h>

emdb_event_mgr_t  emdb_event_mgr;
emdb_connection_t *emdb_cli_conn;
uint8_t           emdb_quit  = 0;
char emdb_serv_ip[255]       = "127.0.0.1";
short int emdb_serv_port     = 8218;
short int emdb_benchmark     = 0;
int       emdb_request_num   = 10000;
int       emdb_parallel_conn = 50; 

static void 
emdb_cli_welcome()
{
  printf("emdb cli, %s\n", EMDB_VERSION);
  printf("Copyright (c) 2014-2015 tinysky\n");
  printf("\n");
}

static void 
emdb_cli_usage(int argc, char **argv)
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
emdb_cli_init_conn()
{
  emdb_event_mgr_init(&emdb_event_mgr);

  if(!emdb_benchmark){
    emdb_cli_conn = emdb_connect_request(emdb_serv_ip, emdb_serv_port);
    if(emdb_cli_conn == NULL){
      fprintf(stderr, "connect to %s:%d failed\n", emdb_serv_ip, emdb_serv_port);
      exit(1);
    }
    emdb_event_mgr_set(&emdb_event_mgr, EMDB_CONN_FD(emdb_cli_conn), FDEVENT_IN, 0, emdb_cli_conn);
  }else{
    int i = 0;
    for(; i < emdb_parallel_conn; ++i){
      emdb_cli_conn = emdb_connect_request(emdb_serv_ip, emdb_serv_port);
      if(emdb_cli_conn == NULL){
         fprintf(stderr, "connect to %s:%d failed\n", emdb_serv_ip, emdb_serv_port);
         exit(1);
      }
      emdb_event_mgr_set(&emdb_event_mgr, EMDB_CONN_FD(emdb_cli_conn), FDEVENT_IN, 0, emdb_cli_conn);      
    } 
  }
}

static void
emdb_cli_init(int argc, char **argv)
{
  if(argc < 3){
    emdb_cli_usage(argc, argv);
    exit(1);
  }
  sprintf(emdb_serv_ip, "%s", argv[1]);
  emdb_serv_port = atoi(argv[2]);

  int pos = 2;
  for(; pos < argc; ++pos){
    if( strncmp(argv[pos], "-b", 2) == 0 ){
      emdb_benchmark = 1;
    }else if( strncmp(argv[pos], "-r", 2) == 0 ){
      int tmp = atoi(argv[pos] + 2);
      if(tmp > 0)  emdb_request_num = tmp;
    }else if( strncmp(argv[pos], "-c", 2) == 0 ){
      int tmp = atoi(argv[pos] + 2);
      if(tmp > 0)  emdb_parallel_conn = tmp;
    }
  }
  
  emdb_cli_init_conn();
}

static void
emdb_cli_cycle()
{
  const emdb_event_t *events = NULL;
  emdb_queue_t *q;

  while(!emdb_quit)
  {
    events = emdb_event_mgr_wait(&emdb_event_mgr, 50);
    if(events == NULL)
    {
      break;
    }    

    q = emdb_queue_next(&(events->queue));
    for(; q != &(events->queue); q = emdb_queue_next(q))
    {
       fprintf(stderr, "ssss\n");
    }

    
  }
}

static void
emdb_exec_benchmark(const char *cmd)
{
  
}

int 
main(int argc, char **argv)
{
  emdb_cli_welcome();
  emdb_cli_init(argc, argv); 
  if(!emdb_benchmark){
     emdb_cli_cycle();    
  }else{
     emdb_exec_benchmark("set");
  }
  return 0;
}
