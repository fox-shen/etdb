#include <etdb.h>

static void
etdb_connect_init_buf(etdb_connection_t *conn)
{
  conn->buf_in  = (etdb_buf_t*)etdb_palloc(conn->pool, sizeof(etdb_buf_t));
  conn->buf_out = (etdb_buf_t*)etdb_palloc(conn->pool, sizeof(etdb_buf_t));
  etdb_buf_init(conn->buf_in,  conn->pool, ETDB_CONNECTION_BUF_SIZE);
  etdb_buf_init(conn->buf_out, conn->pool, ETDB_CONNECTION_BUF_SIZE);

  etdb_queue_init(&(conn->recv_cmd.queue));
  etdb_queue_init(&(conn->free_cmd.queue));
}

void 
etdb_connect_close(etdb_connection_t *conn)
{
  if(conn->sock >= 0)
    close(conn->sock);
  etdb_destroy_pool(conn->pool); 
}

void 
etdb_connect_nodelay(etdb_connection_t *conn, int enable)
{
  int opt = enable ? 1 : 0;
  setsockopt(conn->sock, IPPROTO_TCP, TCP_NODELAY, (void *)&opt, sizeof(opt));
}

void 
etdb_connect_noblock(etdb_connection_t *conn, int enable)
{
  conn->noblock = enable;
  if(enable){
    fcntl(conn->sock, F_SETFL, O_NONBLOCK | O_RDWR);
  }else{
    fcntl(conn->sock, F_SETFL, O_RDWR);
  }
}

void 
etdb_connect_keepalive(etdb_connection_t *conn, int enable)
{
  int opt = enable ? 1 : 0;
  setsockopt(conn->sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&opt, sizeof(opt));
}

etdb_connection_t* 
etdb_connect_request(const char *ip, int port)
{
  etdb_pool_t *pool;
  etdb_connection_t *conn;
  int sock = -1;

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons((short)port);
  inet_pton(AF_INET, ip, &addr.sin_addr);

  if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    goto sock_err;
  }
  if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1){
    goto sock_err;
  }
  pool = etdb_create_pool(ETDB_CONNECTION_DEFAULT_POOL_SIZE);
  conn = etdb_pcalloc(pool, sizeof(etdb_connection_t));
  conn->pool = pool;
  conn->sock = sock;
  etdb_connect_keepalive(conn, 1);
  etdb_connect_init_buf(conn);
  return conn;

sock_err:
  if(sock >= 0)
    close(sock);
  return NULL;
}

etdb_connection_t* 
etdb_connect_listen(const char *ip,  int port)
{
  etdb_connection_t *conn;
  etdb_pool_t *pool;
  int sock = -1;

  int opt = 1;
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons((short)port);
  inet_pton(AF_INET, ip, &addr.sin_addr);

  if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    goto sock_err;
  }
  if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
    goto sock_err;
  }
  if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1){
    goto sock_err;
  }
  if(listen(sock, 1024) == -1){
    goto sock_err;
  }
  pool = etdb_create_pool(ETDB_CONNECTION_DEFAULT_POOL_SIZE);
  conn = etdb_pcalloc(pool, sizeof(etdb_connection_t));
  conn->pool = pool;
  conn->sock = sock;
  snprintf(conn->remote_ip, sizeof(conn->remote_ip), "%s", ip);
  conn->remote_port = port;
  return conn;

sock_err:
  if(sock >= 0)
    close(sock);
  return NULL;  
}

etdb_connection_t* 
etdb_connect_accept(etdb_connection_t *conn)
{
  etdb_pool_t       *pool;
  etdb_connection_t *new_conn;
  int client_sock;
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);

  while((client_sock = accept(conn->sock, (struct sockaddr *)&addr, &addrlen)) == -1){
    if(errno != EINTR){
      return NULL;
    }
  }
  struct linger opt = {1, 0};
  int ret = setsockopt(client_sock, SOL_SOCKET, SO_LINGER, (void *)&opt, sizeof(opt));
  if (ret != 0) {
  }

  pool       = etdb_create_pool(ETDB_CONNECTION_DEFAULT_POOL_SIZE);
  new_conn   = etdb_pcalloc(pool, sizeof(etdb_connection_t));
  new_conn->pool = pool;
  new_conn->sock = client_sock;
  etdb_connect_keepalive(new_conn, 1);
  etdb_connect_init_buf(new_conn);

  inet_ntop(AF_INET, &addr.sin_addr, new_conn->remote_ip, sizeof(new_conn->remote_ip));
  new_conn->remote_port = ntohs(addr.sin_port);
  return new_conn;
}

etdb_bytes_t* 
etdb_connect_alloc_bytes(etdb_connection_t *conn)
{
  etdb_bytes_t *new_bytes = NULL;
  if(etdb_queue_empty(&(conn->free_cmd.queue))){
     new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool, sizeof(etdb_bytes_t));
  }else{
    new_bytes = (etdb_bytes_t*)(conn->free_cmd.queue.next);
    etdb_queue_remove(&(new_bytes->queue));
  }
  return new_bytes;
}

int 
etdb_connect_read_to_buf(etdb_connection_t *conn)
{
  int ret = 0;
  int want;
  etdb_buf_nice(conn->buf_in);
  while((want = etdb_buf_space(conn->buf_in)) > 0){
    int len = read(ETDB_CONN_FD(conn), etdb_buf_slot(conn->buf_in), want);
    if(len == -1){
      if(errno == EINTR)
        continue;
      else if(errno == EWOULDBLOCK)
        break;
      else
        return -1;
    }else{
      if(len == 0)
        return 0;
      ret += len;
      etdb_buf_incr(conn->buf_in, len);
    }
    if(!conn->noblock)
      break;
  }
  return ret;
}

etdb_bytes_t* 
etdb_connect_recv(etdb_connection_t *conn)
{
  if(!etdb_queue_empty(&(conn->recv_cmd.queue))){
    etdb_queue_add(&(conn->free_cmd.queue), &(conn->recv_cmd.queue)); 
    etdb_queue_init(&(conn->recv_cmd.queue));
  }
  if(conn->buf_in->size == 0){
    return &(conn->recv_cmd);
  }
  size_t parsed = 0;
  size_t size   = conn->buf_in->size; 
  uint8_t *head = conn->buf_in->data;
 
  if(head[0] == '*'){ /*** redis protocol  ***/
    if( etdb_resp_recv_redis_req(conn) < 0)
      return NULL;
    else
      return &(conn->recv_cmd);
  }else{
    if( etdb_resp_recv_req(conn) < 0 )
      return NULL;
    else
      return &(conn->recv_cmd); 
  } 
  return NULL;
}

void 
etdb_connect_send_to_buf(etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_queue_t *q = (resp->queue.next); 
  for(; q != &(resp->queue); q = q->next){
    etdb_bytes_t *bytes = (etdb_bytes_t*)q;
    etdb_buf_append_record(conn->buf_out, &(bytes->str));
  }
  etdb_str_t fin_str = etdb_string("\n");
  etdb_buf_append_record(conn->buf_out, &fin_str);
}

int 
etdb_connect_write(etdb_connection_t *conn)
{
  int ret = 0;
  int want;
  while((want = conn->buf_out->size) > 0){
    int len = write(conn->sock, conn->buf_out->data, want);
    if(len == -1){
      if(errno == EINTR){
        continue;
      }else if(errno == EWOULDBLOCK){
        break;
      }else
        return -1;
    }else{
      if(len == 0)
        break;
      ret += len;
      etdb_buf_decr(conn->buf_out, len);
    }
    if(!conn->noblock)
      break;
  } 
  etdb_buf_nice(conn->buf_out);
  return ret;
}


