#include <emdb.h>

static int
emdb_resp_parse_req(emdb_connection_t *conn)
{
  emdb_buf_t*   buf_in    = conn->buf_in;
  emdb_bytes_t* bytes_ptr = &(conn->recv_cmd);

  size_t parsed           = 0;
  size_t size             = buf_in->size;
  uint8_t* ptr            = buf_in->data;
  emdb_bytes_t*  new_bytes;

  int num_args            = 0;
  while(size > 0){
    uint8_t* lf = (uint8_t*)memchr(ptr, '\n', size);
    if(lf == NULL)
      break;
    lf     += 1;
    size   -= (lf - ptr);
    parsed += (lf - ptr);
    
    int len = (int)strtol(ptr + 1, NULL, 10);
    if(errno == EINVAL)
      return -1;
    ptr     = lf;
    if(len < 0)
      return -1;
    if(num_args == 0){
      if(len <= 0)
        return -1;
      num_args = len;
      continue;
    }
    
    ptr    += len + 2;
    size   -= len + 2;
    parsed += len + 2;

    if(emdb_queue_empty(&(conn->free_cmd.queue))){
      new_bytes = (emdb_bytes_t*)emdb_palloc(conn->pool, sizeof(emdb_bytes_t));
    }else{
      new_bytes = (emdb_bytes_t*)(conn->free_cmd.queue.next);
      emdb_queue_remove(&(new_bytes->queue));
    }
    emdb_bytes_set(new_bytes, ptr, len);
    emdb_queue_insert_tail(&(conn->recv_cmd.queue), &(new_bytes->queue));
    
    num_args--;
    if(num_args == 0){
      emdb_buf_decr(buf_in, parsed);
      return 0;
    }
  }

  if(!emdb_queue_empty(&(conn->recv_cmd.queue))){
    emdb_queue_add(&(conn->free_cmd.queue), &(conn->recv_cmd.queue));
    emdb_queue_init(&(conn->recv_cmd.queue));
  }
  return 1;
}

static void
emdb_resp_tolower(emdb_bytes_t* recv_cmd)
{
  recv_cmd = (emdb_bytes_t*)recv_cmd->queue.next;
  emdb_str_tolower(recv_cmd->data, recv_cmd->size);
}

int
emdb_resp_recv_req(emdb_connection_t *conn)
{
  int ret = emdb_resp_parse_req(conn);
  if(ret == -1)
    return -1;

  if(emdb_queue_empty(&(conn->recv_cmd.queue))){
    if(emdb_buf_space(conn->buf_in) == 0){
      emdb_buf_nice(conn->buf_in);
      if(emdb_buf_space(conn->buf_in) == 0){
        if(emdb_buf_grow(conn->buf_in) == -1){
          return -1;
        }
      }
    }
    return 1;
  }
 
  emdb_resp_tolower(&(conn->recv_cmd)); 
}
