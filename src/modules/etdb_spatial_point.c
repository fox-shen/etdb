#include <etdb.h>

static int etdb_sp_set_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_sp_get_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_sp_del_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_sp_rect_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_sp_knn_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_geo_hash_info_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);

#define ETDB_SOID_HEAD  'a'
#define ETDB_SIDX_HEAD  'b'
#define ETDB_GEO_HASH_PRECISION 7

static etdb_command_t etdb_sidx_commands[] = {
  {
    etdb_string("spset"),
    ETDB_CMD_FLAG_ARG3,
    etdb_sp_set_handler,
    etdb_command_padding
  },
  {
    etdb_string("spget"),
    ETDB_CMD_FLAG_ARG1,
    etdb_sp_get_handler,
    etdb_command_padding
  },
  {
    etdb_string("spdel"),
    ETDB_CMD_FLAG_ARG1,
    etdb_sp_del_handler,
    etdb_command_padding
  },
  {
    etdb_string("sprect"),
    ETDB_CMD_FLAG_ARG4,
    etdb_sp_rect_handler,
    etdb_command_padding    
  },
  {
    etdb_string("spknn"),
    ETDB_CMD_FLAG_ARG2|ETDB_CMD_FLAG_ARG3,
    etdb_sp_knn_handler,
    etdb_command_padding 
  },
  {
    etdb_string("geo_hash_info"),
    ETDB_CMD_FLAG_NOARG,
    etdb_geo_hash_info_handler,
    etdb_command_padding
  },
  {
    etdb_null_command
  }
};

etdb_module_t etdb_sidx_module={
  etdb_string("spatial point"),
  etdb_sidx_commands,
  NULL, 
  etdb_module_padding
};

static int 
etdb_sp_set_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *key     = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *lat     = (etdb_bytes_t*)(key->queue.next);
  etdb_bytes_t *lon     = (etdb_bytes_t*)(lat->queue.next);

  return etdb_database_sp_set(conn->slot, &key->str, &lat->str, &lon->str, conn->pool_temp);
}

static int 
etdb_sp_get_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *key     = (etdb_bytes_t*)(args->queue.next->next);
  double lat, lon;
  char geo_hash_code[ETDB_GEO_HASH_PRECISION_LEN + 1] = "\0";
  if(etdb_database_sp_get(conn->slot, &key->str, &lat, &lon, geo_hash_code) < 0) return -1; 

  char temp[64];
  sprintf(temp, "%.2f, %.2f (%s)", lat, lon, geo_hash_code);

  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool_temp,
                                               sizeof(etdb_bytes_t) + strlen(temp));
  new_bytes->str.len    =  strlen(temp);
  new_bytes->str.data   =  (uint8_t*)new_bytes + sizeof(etdb_bytes_t);
  memcpy(new_bytes->str.data, temp, new_bytes->str.len);
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));
  return 0; 
}

static int 
etdb_sp_del_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *key     = (etdb_bytes_t*)(args->queue.next->next);
  return etdb_database_sp_del(conn->slot, &key->str, conn->pool_temp);
}

static int 
etdb_sp_rect_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *lat1    = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *lat2    = (etdb_bytes_t*)(lat1->queue.next);
  etdb_bytes_t *lon1    = (etdb_bytes_t*)(lat2->queue.next);
  etdb_bytes_t *lon2    = (etdb_bytes_t*)(lon1->queue.next);

  return etdb_database_sp_rect(conn->slot, &lat1->str, &lat2->str, 
                               &lon1->str, &lon2->str, conn->pool_temp, resp); 
}

static int 
etdb_sp_knn_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool_temp, sizeof(etdb_bytes_t));
  new_bytes->str.len    =  strlen("knn");
  new_bytes->str.data   =  "knn";
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));
  return 0;
}

static int 
etdb_geo_hash_info_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  char temp[256];
  etdb_geo_hash_box_dimension_t dim = etdb_geo_hash_dimensions_for_precision(ETDB_GEO_HASH_PRECISION_LEN);
  sprintf(temp, "range: %f %f", dim.width, dim.height);

  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc(conn->pool_temp, 
                                               sizeof(etdb_bytes_t) + strlen(temp));
  new_bytes->str.data    =  (uint8_t*)new_bytes + sizeof(etdb_bytes_t);
  new_bytes->str.len     =  strlen(temp);
  memcpy(new_bytes->str.data, temp, new_bytes->str.len);
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));
  return 0;
}
