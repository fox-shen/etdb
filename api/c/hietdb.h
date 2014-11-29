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
etdb_status_t etdb_set(etdb_client_t *client, etdb_str_t *key, etdb_str_t *value);
etdb_status_t etdb_get(etdb_client_t *client, etdb_str_t *key, etdb_str_t *value);


#endif
