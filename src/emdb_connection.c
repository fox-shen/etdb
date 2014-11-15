#include <emdb.h>

void 
emdb_connect_close(emdb_connection_t *conn)
{
  if(conn->sock >= 0)
    close(conn->sock);
  emdb_destroy_pool(conn->pool); 
}

void 
emdb_connect_nodelay(emdb_connection_t *conn, int enable)
{
  int opt = enable ? 1 : 0;
  setsockopt(conn->sock, IPPROTO_TCP, TCP_NODELAY, (void *)&opt, sizeof(opt));
}

void 
emdb_connect_noblock(emdb_connection_t *conn, int enable)
{
  conn->noblock = enable;
  if(enable){
    fcntl(conn->sock, F_SETFL, O_NONBLOCK | O_RDWR);
  }else{
    fcntl(conn->sock, F_SETFL, O_RDWR);
  }
}

void 
emdb_connect_keepalive(emdb_connection_t *conn, int enable)
{
  int opt = enable ? 1 : 0;
  setsockopt(conn->sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&opt, sizeof(opt));
}

emdb_connection_t* 
emdb_connect_request(const char *ip, int port)
{
  emdb_pool_t *pool;
  emdb_connection_t *conn;
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
  pool = emdb_create_pool(EMDB_CONNECTION_DEFAULT_POOL_SIZE);
  conn = emdb_palloc(pool, sizeof(emdb_connection_t));
  conn->pool = pool;
  conn->sock = sock;
  emdb_connect_keepalive(conn, 1);
  return conn;

sock_err:
  if(sock >= 0)
    close(sock);
  return NULL;
}

emdb_connection_t* 
emdb_connect_listen(const char *ip,  int port)
{
  emdb_connection_t *conn;
  emdb_pool_t *pool;
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
  pool = emdb_create_pool(EMDB_CONNECTION_DEFAULT_POOL_SIZE);
  conn = emdb_palloc(pool, sizeof(emdb_connection_t));
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

emdb_connection_t* 
emdb_connect_accept(emdb_connection_t *conn)
{
  emdb_pool_t       *pool;
  emdb_connection_t *new_conn;
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

  pool       = emdb_create_pool(EMDB_CONNECTION_DEFAULT_POOL_SIZE);
  new_conn   = emdb_palloc(pool, sizeof(emdb_connection_t));
  new_conn->pool = pool;
  new_conn->sock = client_sock;
  emdb_connect_keepalive(new_conn, 1);

  inet_ntop(AF_INET, &addr.sin_addr, new_conn->remote_ip, sizeof(new_conn->remote_ip));
  new_conn->remote_port = ntohs(addr.sin_port);
  return new_conn;
}
