#include <etdb.h>

extern etdb_event_mgr_t  etdb_event_mgr;

static int
etdb_serv_parse_args(etdb_bytes_t *req, etdb_command_t *cmd)
{
  int cmd_cnt = etdb_queue_count(&(req->queue)) - 1;
  switch(cmd_cnt){
    case 0:
      if((cmd->flags & ETDB_CMD_FLAG_NOARG) == 0) return -1;
      break;
    case 1:
      if((cmd->flags & ETDB_CMD_FLAG_ARG1) == 0)  return -1;
      break;
    case 2:
      if((cmd->flags & ETDB_CMD_FLAG_ARG2) == 0)  return -1;
      break;
    case 3:
      if((cmd->flags & ETDB_CMD_FLAG_ARG3) == 0)  return -1;
      break;
    case 4:
      if((cmd->flags & ETDB_CMD_FLAG_ARG4) == 0)  return -1;
      break;
    default:
      break;
  }
  return 0;
}

void 
etdb_serv_exec_proc(etdb_bytes_t *req, etdb_connection_t *conn)
{
  etdb_command_t* cmd = etdb_module_find_command(&(((etdb_bytes_t*)(req->queue.next))->str)); 
  etdb_bytes_t resp;
  etdb_queue_init(&(resp.queue));

  if(cmd == NULL || etdb_serv_parse_args(req, cmd) < 0){
    etdb_bytes_t n;
    size_t len = sizeof("Unkown Command:") - 1 + (((etdb_bytes_t*)(req->queue.next))->str).len + etdb_bytes_total_len(req);
    char *head = etdb_palloc_temp(conn->pool_temp, len);
    char *pos  = memcpy(head, "Unkown Command:", sizeof("Unkown Command:") - 1) + sizeof("Unkown Command:") - 1;
    etdb_bytes_total_copy(pos, req);
    
    etdb_bytes_set(&n, head, len);
    etdb_queue_insert_tail(&(resp.queue), &(n.queue));

    etdb_connect_send_to_buf(conn, &resp);
  }else{
    int ret = cmd->handler(req, conn, &resp);
    if(cmd->flags & ETDB_CMD_FLAG_WRITE){
       etdb_bytes_t n;
       if(ret == 0){ 
         etdb_bytes_set(&n, "+OK", sizeof("+OK") - 1);
       }else{
         etdb_bytes_set(&n, "+FAIL", sizeof("+FAIL") - 1);
       }
       etdb_queue_insert_tail(&(resp.queue), &(n.queue));
       etdb_connect_send_to_buf(conn, &resp); 
    }else{
      etdb_bytes_t n;
      if(ret < 0){
        etdb_bytes_set(&n, "+Not Found", sizeof("+Not Found") -1);
        etdb_queue_insert_tail(&(resp.queue), &(n.queue));
      }
      etdb_connect_send_to_buf(conn, &resp); 
    }   
  }

  if(conn->buf_out->size > 0){
    etdb_event_mgr_set(&etdb_event_mgr, ETDB_CONN_FD(conn), FDEVENT_OUT, 1, conn);
  } 
}
