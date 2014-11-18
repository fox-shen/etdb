#ifndef H_ETDB_RESP_H
#define H_ETDB_RESP_H

int etdb_resp_recv_redis_req(etdb_connection_t *conn);
int etdb_resp_recv_req(etdb_connection_t *conn);

#endif
