#ifndef H_HIETDB_H
#define H_HIETDB_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stddef.h>

#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <etdb_queue.h>
#include <etdb_string.h>
#include <etdb_bytes.h>

typedef struct etdb_client_s{
  char    host[32];
  int     port;
  void    *link;
  uint8_t *command_buf;
  int     command_buf_len;
}etdb_client_t;

typedef etdb_str_t etdb_status_t;

int etdb_client_connect(etdb_client_t *client, const char *host, int port);

/**** etdb api *****/
/*** db: type k-v ***/
etdb_status_t 
etdb_kv_set(etdb_client_t *client, etdb_str_t *key, etdb_str_t *value);

etdb_status_t 
etdb_kv_get(etdb_client_t *client, etdb_str_t *key, etdb_str_t *value);

etdb_status_t 
etdb_kv_del(etdb_client_t *client, etdb_str_t *key);

etdb_status_t 
etdb_kv_matchlongest(etdb_client_t *client, etdb_str_t *key, etdb_bytes_t **list);


/*** db: type hash ***/
etdb_status_t 
etdb_hash_set(etdb_client_t *client, etdb_str_t *tb, etdb_str_t *key, etdb_str_t *value);

etdb_status_t 
etdb_hash_get(etdb_client_t *client, etdb_str_t *tb, etdb_str_t *key, etdb_str_t *value);

etdb_status_t
etdb_hash_del(etdb_client_t *client, etdb_str_t *tb, etdb_str_t *key);

/*** db: type set ***/
etdb_status_t
etdb_set_add(etdb_client_t *client, etdb_str_t *sn, etdb_str_t *item);
etdb_status_t
etdb_set_del(etdb_client_t *client, etdb_str_t *sn, etdb_str_t *item);
etdb_status_t
etdb_set_members(etdb_client_t *client, etdb_str_t *sn, etdb_bytes_t **value);
etdb_status_t
etdb_set_ismember(etdb_client_t *client, etdb_str_t *sn, etdb_str_t *item);

/*** db: type list ***/
etdb_status_t
etdb_lst_lpush(etdb_client_t *client, etdb_str_t *ln, etdb_str_t *item);
etdb_status_t
etdb_lst_rpush(etdb_client_t *client, etdb_str_t *ln, etdb_str_t *item);
etdb_status_t
etdb_lst_ltop(etdb_client_t *client, etdb_str_t *ln, etdb_str_t *value);
etdb_status_t
etdb_lst_rtop(etdb_client_t *client, etdb_str_t *ln, etdb_str_t *value);
etdb_status_t
etdb_lst_lpop(etdb_client_t *client, etdb_str_t *ln);
etdb_status_t
etdb_lst_rpop(etdb_client_t *client, etdb_str_t *ln);

/*** db: type spatial point ***/
etdb_status_t
etdb_spatial_point_set(etdb_client_t *client, etdb_str_t *id, double lat,  double lon);
etdb_status_t
etdb_spatial_point_get(etdb_client_t *client, etdb_str_t *id, double *lat, double *lon, char *code, int *code_len);
etdb_status_t
etdb_spatial_point_rect_query(etdb_client_t *client, double lat1, double lat2, double lon1, double lon2, etdb_bytes_t **value_list);



#endif
