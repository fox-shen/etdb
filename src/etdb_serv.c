#include <etdb.h>

extern etdb_event_mgr_t  etdb_event_mgr;

void 
etdb_serv_exec_proc(etdb_bytes_t *req, etdb_connection_t *conn)
{
  etdb_command_t* cmd = etdb_module_find_command(&(((etdb_bytes_t*)(req->queue.next))->str)); 
  if(cmd == NULL){
    etdb_bytes_t resp, n;
    char tmp_data[128];
    etdb_queue_init(&(resp.queue));
    etdb_bytes_set(&n, "Unkown Command:", sizeof("Unkown Command:") - 1);
    etdb_queue_insert_tail(&(resp.queue), &(n.queue));

    etdb_connect_send_to_buf(conn, &resp); 
    size_t space = etdb_buf_space(conn->buf_out);
fprintf(stderr, "recv command\n");
  }else{
    
  }

  if(conn->buf_out->size > 0){
    etdb_event_mgr_set(&etdb_event_mgr, ETDB_CONN_FD(conn), FDEVENT_OUT, 1, conn);
  } 
}
