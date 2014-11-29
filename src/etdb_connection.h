#ifndef H_ETDB_CONNECTION_H
#define H_ETDB_CONNECTION_H

#define ETDB_CONNECTION_BUF_SIZE 8192

typedef struct etdb_connection_s etdb_connection_t;
struct etdb_connection_s{
  etdb_queue_t queue;
  int     sock;
  int     noblock;
  int     error;

  char    remote_ip[INET_ADDRSTRLEN];
  int     remote_port;  

  etdb_pool_t *pool; 
  etdb_pool_t *pool_temp;

  etdb_buf_t  *buf_in;
  etdb_buf_t  *buf_out;

  etdb_bytes_t recv_cmd;
  etdb_bytes_t free_cmd;
};

#define ETDB_CONNECTION_DEFAULT_POOL_SIZE 8192

void etdb_connect_close(etdb_connection_t *conn);
void etdb_connect_nodelay(etdb_connection_t *conn, int enable);
void etdb_connect_noblock(etdb_connection_t *conn, int enable);
void etdb_connect_keepalive(etdb_connection_t *conn, int enable);

etdb_connection_t* etdb_connect_request(const char *ip, int port);
etdb_connection_t* etdb_connect_listen(const char *ip,  int port);
etdb_connection_t* etdb_connect_accept(etdb_connection_t *conn);
#define ETDB_CONN_FD(conn) (conn->sock)
etdb_bytes_t* etdb_connect_alloc_bytes(etdb_connection_t *conn);

int etdb_connect_read_to_buf(etdb_connection_t *conn);
etdb_bytes_t* etdb_connect_recv(etdb_connection_t *conn);
void etdb_connect_send_to_buf(etdb_connection_t *conn, etdb_bytes_t *resp);
int etdb_connect_write(etdb_connection_t *conn);
int etdb_connect_send_cmd(etdb_connection_t *conn, uint8_t *cmd, size_t len);

int etdb_connect_flush(etdb_connection_t *conn);

#endif
