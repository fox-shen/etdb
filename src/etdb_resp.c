#include <etdb.h>

static int
etdb_resp_parse_redis_req(etdb_connection_t *conn)
{
  etdb_buf_t*   buf_in    = conn->buf_in;
  etdb_bytes_t* bytes_ptr = &(conn->recv_cmd);

  size_t parsed           = 0;
  size_t size             = buf_in->size;
  uint8_t* ptr            = buf_in->data;
  etdb_bytes_t*  new_bytes;

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

    if(etdb_queue_empty(&(conn->free_cmd.queue))){
      new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool, sizeof(etdb_bytes_t));
    }else{
      new_bytes = (etdb_bytes_t*)(conn->free_cmd.queue.next);
      etdb_queue_remove(&(new_bytes->queue));
    }
    etdb_bytes_set(new_bytes, ptr, len);
    etdb_queue_insert_tail(&(conn->recv_cmd.queue), &(new_bytes->queue));
    
    num_args--;
    if(num_args == 0){
      etdb_buf_decr(buf_in, parsed);
      return 0;
    }
  }

  if(!etdb_queue_empty(&(conn->recv_cmd.queue))){
    etdb_queue_add(&(conn->free_cmd.queue), &(conn->recv_cmd.queue));
    etdb_queue_init(&(conn->recv_cmd.queue));
  }
  return 1;
}

static int
etdb_resp_parse_req(etdb_connection_t *conn)
{
  etdb_buf_t*   buf_in    = conn->buf_in;
  etdb_bytes_t* bytes_ptr = &(conn->recv_cmd);

  size_t parsed           = 0;
  size_t size             = buf_in->size;
  uint8_t* ptr            = buf_in->data;
  etdb_bytes_t*  new_bytes;

  while(size > 0 && (ptr[0] == '\n' || ptr[0] == '\r')){
    
  } 
}

static void
etdb_resp_tolower(etdb_bytes_t* recv_cmd)
{
  recv_cmd = (etdb_bytes_t*)recv_cmd->queue.next;
  etdb_str_tolower(recv_cmd->data, recv_cmd->size);
}

int
etdb_resp_recv_req(etdb_connection_t *conn)
{
  int ret = etdb_resp_parse_req(conn);
  if(ret == -1)
    return -1;

  if(etdb_queue_empty(&(conn->recv_cmd.queue))){
    if(etdb_buf_space(conn->buf_in) == 0){
      etdb_buf_nice(conn->buf_in);
      if(etdb_buf_space(conn->buf_in) == 0){
        if(etdb_buf_grow(conn->buf_in) == -1){
          return -1;
        }
      }
    }
    return 1;
  }
 
  etdb_resp_tolower(&(conn->recv_cmd)); 
}
