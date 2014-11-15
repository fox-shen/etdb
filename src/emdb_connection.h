#ifndef H_EMDB_CONNECTION_H
#define H_EMDB_CONNECTION_H

typedef struct emdb_connection_s emdb_connection_t;
struct emdb_connection_s{
  int sock;
  int noblock;
  int error;
  
};

#endif
