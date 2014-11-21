#include <etdb.h>

#define ETDB_MAX_PACKET_SIZE 32*1024*1024

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

    new_bytes = etdb_connect_alloc_bytes(conn);
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
  uint8_t *head           = buf_in->data;
  etdb_bytes_t*  new_bytes;

  while(size > 0 && (head[0] == '\n')){
    ++head;
    --size;
    ++parsed; 
  }

#ifdef DEBUG
  etdb_buf_dsp(buf_in);
#endif

  while(size > 0){
    uint8_t *body = (uint8_t*)memchr(head, '\n', size);
    if(body == NULL)
      break;
    ++body;
    int head_len = body - head;
    if(head[0] < '0' || head[0] > '9')   return -1; 
    char head_str[16];
    if(head_len > (int)sizeof(head_str) - 1)  return -1;

    memcpy(head_str, head, head_len - 1);
    head_str[head_len - 1] = '\0';

    int body_len = atoi(head_str);
    if(body_len < 0)   return -1;

    size -= head_len + body_len;
    if(size < 0)  break;

    if(body[0] == '\n'){
      parsed += head_len + body_len;
      etdb_buf_decr(buf_in, parsed);
      return 0;
    }

    etdb_bytes_t *new_bytes = etdb_connect_alloc_bytes(conn);
    etdb_bytes_set(new_bytes, body, body_len);
    etdb_queue_insert_tail(&(conn->recv_cmd.queue), &(new_bytes->queue)); 

    head += head_len + body_len;
    parsed += head_len + body_len;
    if(size > 0 && head[0] == '\n'){
      head   += 1;
      size   -= 1;
      parsed += 1;
    }else
      break;
        
    if(parsed > ETDB_MAX_PACKET_SIZE)
      return -1;
  }
  if(!etdb_queue_empty(&(conn->recv_cmd.queue))){
    etdb_queue_add(&(conn->free_cmd.queue), &(conn->recv_cmd.queue));
    etdb_queue_init(&(conn->recv_cmd.queue));
  }
  return 1;
}

void
etdb_resp_tolower(etdb_bytes_t* recv_cmd)
{
  recv_cmd = (etdb_bytes_t*)recv_cmd->queue.next;
  etdb_str_tolower(recv_cmd->str.data, recv_cmd->str.len);
}

int
etdb_resp_recv_redis_req(etdb_connection_t *conn)
{
  int ret = etdb_resp_parse_redis_req(conn);
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
}

