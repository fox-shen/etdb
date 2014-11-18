#include <emdb.h>

extern emdb_event_mgr_t  emdb_event_mgr;

void 
emdb_serv_exec_proc(emdb_bytes_t *req, emdb_connection_t *conn)
{
  emdb_command_t* cmd = emdb_module_find_command((emdb_bytes_t*)(req->queue.next)); 
  if(cmd == NULL){
    emdb_bytes_t resp, n;
    emdb_queue_init(&(resp.queue));
    emdb_bytes_set(&n, "Unkown Command:", sizeof("Unkown Command:") - 1);
    emdb_queue_insert_tail(&(resp.queue), &(n.queue));

    emdb_connect_send(conn, &resp); 
    size_t space = emdb_buf_space(conn->buf_out);
    fprintf(stderr, "space size: %d\n", conn->buf_out->size);
  }else{
    
  }

  if(conn->buf_out->size > 0){
    emdb_event_mgr_set(&emdb_event_mgr, EMDB_CONN_FD(conn), FDEVENT_OUT, 1, conn);
  } 
}
