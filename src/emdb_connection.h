#ifndef H_EMDB_CONNECTION_H
#define H_EMDB_CONNECTION_H

#define EMDB_CONNECTION_BUF_SIZE 8192

typedef struct emdb_connection_s emdb_connection_t;
struct emdb_connection_s{
  emdb_queue_t queue;
  int     sock;
  int     noblock;
  int     error;

  char    remote_ip[INET_ADDRSTRLEN];
  int     remote_port;  

  emdb_pool_t *pool; 

  emdb_buf_t  *buf_in;
  emdb_buf_t  *buf_out;

  emdb_bytes_t recv_data;
};

#define EMDB_CONNECTION_DEFAULT_POOL_SIZE 8192

void emdb_connect_close(emdb_connection_t *conn);
void emdb_connect_nodelay(emdb_connection_t *conn, int enable);
void emdb_connect_noblock(emdb_connection_t *conn, int enable);
void emdb_connect_keepalive(emdb_connection_t *conn, int enable);

emdb_connection_t* emdb_connect_request(const char *ip, int port);
emdb_connection_t* emdb_connect_listen(const char *ip,  int port);
emdb_connection_t* emdb_connect_accept(emdb_connection_t *conn);
#define EMDB_CONN_FD(conn) (conn->sock)

int emdb_connect_read(emdb_connection_t *conn);
emdb_bytes_t* emdb_connect_recv(emdb_connection_t *conn);

#endif
