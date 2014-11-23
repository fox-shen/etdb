#include <etdb.h>

static int etdb_sidx_sset_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_sidx_sget_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_sidx_srect_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_sidx_sknn_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);

#define ETDB_SOID_HEAD  'a'
#define ETDB_SIDX_HEAD  'b'
#define ETDB_GEO_HASH_PRECISION 12

static etdb_command_t etdb_sidx_commands[] = {
  {
    etdb_string("sset"),
    ETDB_CMD_FLAG_ARG3,
    etdb_sidx_sset_handler,
    etdb_command_padding
  },
  {
    etdb_string("sget"),
    ETDB_CMD_FLAG_ARG1,
    etdb_sidx_sget_handler,
    etdb_command_padding
  },
  {
    etdb_string("srect"),
    ETDB_CMD_FLAG_ARG4,
    etdb_sidx_srect_handler,
    etdb_command_padding    
  },
  {
    etdb_string("sknn"),
    ETDB_CMD_FLAG_ARG2,
    etdb_sidx_sknn_handler,
    etdb_command_padding 
  },
  {
    etdb_null_command
  }
};

etdb_module_t etdb_sidx_module={
  etdb_string("sidx"),
  etdb_sidx_commands,
  NULL, 
  etdb_module_padding
};

static int 
etdb_sidx_sset_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *key     = (etdb_bytes_t*)(args->queue.next->next);
  etdb_bytes_t *lat     = (etdb_bytes_t*)(key->queue.next);
  etdb_bytes_t *lon     = (etdb_bytes_t*)(lat->queue.next);

  int ret = 0;
  double lat_d, lon_d;
  if(etdb_atof(lat->str.data, lat->str.len, &lat_d) == -1 ||
     etdb_atof(lon->str.data, lon->str.len, &lon_d) == -1 ||
     lat_d < -90 || lat_d > 90 || lon_d < -180 || lon_d > 180) 
  {
    return -1;
  }
  uint8_t *value;
  size_t  value_len;  

  /*** step1: set (SID -> LON LAT) value ***/ 
  *(key->str.data - 1)  = ETDB_SOID_HEAD;
  ret = etdb_database_exact_match(key->str.data - 1, key->str.len + 1, &value, &value_len);

  if(ret < 0){ 
    double values[] = {lat_d, lon_d};
    ret = etdb_database_update(key->str.data - 1, key->str.len + 1, (uint8_t*)values, sizeof(values));
    if(ret != 0)  return ret;
  }else{
    double *pd =  (double*)value;
    char hash_code[ETDB_GEO_HASH_PRECISION + 2];
    hash_code[ETDB_GEO_HASH_PRECISION + 1] = '\0';
    hash_code[0]   = ETDB_SIDX_HEAD;
    if(etdb_geo_hash_encode(*pd, *(pd + 1), hash_code + 1, ETDB_GEO_HASH_PRECISION) == NULL)
      return -1;
    *pd        =  lat_d;
    *(pd + 1)  =  lon_d;
    if(etdb_database_list_remove(hash_code, ETDB_GEO_HASH_PRECISION + 1, 
                                 key->str.data, key->str.len) != 0)
      return -1;
  }

  /*** step2: set (LON LAT -> SID) value ***/
  char hash_code[ETDB_GEO_HASH_PRECISION + 2];
  hash_code[ETDB_GEO_HASH_PRECISION + 1] = '\0';
  hash_code[0]   = ETDB_SIDX_HEAD;
  if(etdb_geo_hash_encode(lat_d, lon_d, hash_code + 1, ETDB_GEO_HASH_PRECISION) == NULL)  
    return -1;
  ret = etdb_database_list_rpush(hash_code, ETDB_GEO_HASH_PRECISION + 1, key->str.data, key->str.len);
  if(ret != 0)  return ret; 
  return ret;
}

static int 
etdb_sidx_sget_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  etdb_bytes_t *key     = (etdb_bytes_t*)(args->queue.next->next);
  *(key->str.data - 1)  = ETDB_SOID_HEAD;

  uint8_t *value = NULL;
  size_t value_len = 0;
  int ret = etdb_database_exact_match(key->str.data - 1, key->str.len + 1, &value, &value_len); 

  if(ret < 0){
     return -1; 
  }
  char temp[256];
  double *pd = (double*)value;
  sprintf(temp, "%f, %f", *pd, *(pd + 1));

  etdb_bytes_t *new_bytes = (etdb_bytes_t*)etdb_palloc_temp(conn->pool_temp,
                                               sizeof(etdb_bytes_t) + strlen(temp));
  new_bytes->str.len    =  strlen(temp);
  new_bytes->str.data   =  (uint8_t*)new_bytes + sizeof(etdb_bytes_t);
  memcpy(new_bytes->str.data, temp, new_bytes->str.len);
  etdb_queue_insert_tail(&(resp->queue), &(new_bytes->queue));
  return 0; 
}

static int 
etdb_sidx_srect_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  return 0;
}

static int 
etdb_sidx_sknn_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp)
{
  return 0;
}
