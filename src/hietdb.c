#include <etdb.h>
#include <hietdb.h>

int 
etdb_client_connect(etdb_client_t *client, const char *host, int port)
{
  strcpy(client->host, host);
  client->port  = port;
  client->link  = etdb_connect_request(client->host, client->port); 
  if(client->link == NULL)  return -1;
  client->command_buf_len = 32;
  client->command_buf     = (char*)etdb_alloc(client->command_buf_len);
  return 0;
}

etdb_bytes_t*
etdb_client_request(etdb_client_t *client)
{
  if(etdb_connect_flush(client->link) == -1)   return NULL;

  while(1){
    etdb_bytes_t *resp = etdb_connect_recv(client->link);
    if(resp == NULL){
      return NULL;
    }else if(etdb_queue_empty(&(resp->queue))){
      if(etdb_connect_read_to_buf(client->link) <= 0){
        return NULL;
      }
    }else{
      return resp;
    } 
  }
  return NULL; 
}

#define etdb_cli_conn(cl) ((etdb_connection_t*)(cl->link))

/*** db: kv type implementation ***/

etdb_status_t 
etdb_kv_set(etdb_client_t *client, etdb_str_t *key, etdb_str_t *value)
{
  etdb_status_t status = etdb_string("+ERROR");
  etdb_str_t    set_cmd= etdb_string("set"); 

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &set_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, key);  
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, value);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);
  etdb_bytes_t* resp = etdb_client_request(client); 

  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status           = bv->str;
  }
  return status; 
}

etdb_status_t 
etdb_kv_get(etdb_client_t *client, etdb_str_t *key, etdb_str_t *value)
{
  etdb_status_t status = etdb_string("+ERROR");
  etdb_str_t    get_cmd= etdb_string("get");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &get_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, key);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);

  etdb_bytes_t* resp = etdb_client_request(client); 
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status           = bv->str;
    if(bv->queue.next != &(resp->queue)){
      bv             = (etdb_bytes_t*)(bv->queue.next);
      *value         = bv->str;
    }
  }
  return status; 
}

etdb_status_t 
etdb_kv_del(etdb_client_t *client, etdb_str_t *key)
{
  etdb_status_t status = etdb_string("+ERROR");
  etdb_str_t    del_cmd= etdb_string("del");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &del_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, key);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);

  etdb_bytes_t* resp = etdb_client_request(client);
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status           = bv->str;
  }
  return status;
}

etdb_status_t 
etdb_kv_matchlongest(etdb_client_t *client, etdb_str_t *key, etdb_bytes_t **value)
{
  etdb_status_t status  = etdb_string("+ERROR");
  etdb_str_t    match_longest_cmd = etdb_string("matchlongest");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &match_longest_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, key);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out); 

  etdb_bytes_t *resp = etdb_client_request(client);
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    *value = resp;
    status = bv->str;
  }
  return status;
}

/*** db: type hash implementation***/
etdb_status_t
etdb_hash_set(etdb_client_t *client, etdb_str_t *tb, etdb_str_t *key, etdb_str_t *value)
{
  etdb_status_t status = etdb_string("+ERROR");
  etdb_str_t    set_cmd= etdb_string("hset");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &set_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, tb);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, key);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, value);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);
  etdb_bytes_t* resp = etdb_client_request(client);

  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status           = bv->str;
  }
  return status;
}

etdb_status_t
etdb_hash_get(etdb_client_t *client, etdb_str_t *tb, etdb_str_t *key, etdb_str_t *value)
{
  etdb_status_t status = etdb_string("+ERROR");
  etdb_str_t    get_cmd= etdb_string("hget");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &get_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, tb);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, key);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);

  etdb_bytes_t* resp = etdb_client_request(client);
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status           = bv->str;
    if(bv->queue.next != &(resp->queue)){
      bv             = (etdb_bytes_t*)(bv->queue.next);
      *value         = bv->str;
    }
  }
  return status;
}

etdb_status_t
etdb_hash_del(etdb_client_t *client, etdb_str_t *tb, etdb_str_t *key)
{
  etdb_status_t status = etdb_string("+ERROR");
  etdb_str_t    del_cmd= etdb_string("hdel");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &del_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, tb);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, key);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);

  etdb_bytes_t* resp = etdb_client_request(client);
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status           = bv->str;
  }
  return status;
}

/*** db: type set ***/
etdb_status_t
etdb_set_add(etdb_client_t *client, etdb_str_t *sn, etdb_str_t *item)
{
  etdb_status_t status = etdb_string("+ERROR");
  etdb_str_t    set_cmd= etdb_string("sadd");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &set_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, sn);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, item);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);
  etdb_bytes_t* resp = etdb_client_request(client);

  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status           = bv->str;
  }
  return status;
}

etdb_status_t
etdb_set_del(etdb_client_t *client, etdb_str_t *sn, etdb_str_t *item)
{
  etdb_status_t status = etdb_string("+ERROR");
  etdb_str_t    del_cmd= etdb_string("sdel");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &del_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, sn);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, item);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);

  etdb_bytes_t* resp = etdb_client_request(client);
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status           = bv->str;
  }
  return status;
}

etdb_status_t
etdb_set_members(etdb_client_t *client, etdb_str_t *sn, etdb_bytes_t **value)
{
  etdb_status_t status  = etdb_string("+ERROR");
  etdb_str_t    iter_cmd = etdb_string("smembers");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &iter_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, sn);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);

  etdb_bytes_t *resp = etdb_client_request(client);
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    *value = resp;
    status = bv->str;
  }
  return status;
}

etdb_status_t
etdb_set_ismember(etdb_client_t *client, etdb_str_t *sn, etdb_str_t *item)
{
  etdb_status_t status  = etdb_string("+ERROR");
  etdb_str_t    ismember_cmd = etdb_string("sismember");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &ismember_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, sn);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, item);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);

  etdb_bytes_t *resp = etdb_client_request(client);
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status = bv->str;
  }
  return status;
}

/*** db: type list ***/
etdb_status_t
etdb_lst_lpush(etdb_client_t *client, etdb_str_t *ln, etdb_str_t *item)
{
  etdb_status_t status  = etdb_string("+ERROR");
  etdb_str_t    lpush_cmd = etdb_string("lpush");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &lpush_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, ln);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, item);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);

  etdb_bytes_t *resp = etdb_client_request(client);
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status = bv->str;
  }
  return status; 
}

etdb_status_t
etdb_lst_rpush(etdb_client_t *client, etdb_str_t *ln, etdb_str_t *item)
{
  etdb_status_t status  = etdb_string("+ERROR");
  etdb_str_t    rpush_cmd = etdb_string("rpush");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &rpush_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, ln);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, item);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);

  etdb_bytes_t *resp = etdb_client_request(client);
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status = bv->str;
  }
  return status;
}

etdb_status_t
etdb_lst_ltop(etdb_client_t *client, etdb_str_t *ln, etdb_str_t *value)
{
  etdb_status_t status  = etdb_string("+ERROR");
  etdb_str_t    ltop_cmd = etdb_string("ltop");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &ltop_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, ln);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);

  etdb_bytes_t *resp = etdb_client_request(client);
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status           = bv->str;
    if(bv->queue.next != &(resp->queue)){
      bv             = (etdb_bytes_t*)(bv->queue.next);
      *value         = bv->str;
    }
  }
  return status;
}

etdb_status_t
etdb_lst_rtop(etdb_client_t *client, etdb_str_t *ln, etdb_str_t *value)
{
  etdb_status_t status  = etdb_string("+ERROR");
  etdb_str_t    ltop_cmd = etdb_string("rtop");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &ltop_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, ln);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);

  etdb_bytes_t *resp = etdb_client_request(client);
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status           = bv->str;
    if(bv->queue.next != &(resp->queue)){
      bv             = (etdb_bytes_t*)(bv->queue.next);
      *value         = bv->str;
    }
  }
  return status; 
}

etdb_status_t
etdb_lst_lpop(etdb_client_t *client, etdb_str_t *ln)
{
  etdb_status_t status  = etdb_string("+ERROR");
  etdb_str_t    rpush_cmd = etdb_string("lpop");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &rpush_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, ln);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);

  etdb_bytes_t *resp = etdb_client_request(client);
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status = bv->str;
  }
  return status;
}

etdb_status_t
etdb_lst_rpop(etdb_client_t *client, etdb_str_t *ln)
{
  etdb_status_t status  = etdb_string("+ERROR");
  etdb_str_t    rpush_cmd = etdb_string("rpop");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &rpush_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, ln);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);

  etdb_bytes_t *resp = etdb_client_request(client);
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status = bv->str;
  }
  return status;
}

/*** db: type spatial point ***/
etdb_status_t
etdb_spatial_point_set(etdb_client_t *client, etdb_str_t *id, double lat,  double lon)
{
  char lat_str[128], lon_str[128];
  sprintf(lat_str, "%f", lat);
  sprintf(lon_str, "%f", lon);

  etdb_str_t lat_in = {strlen(lat_str), lat_str};
  etdb_str_t lon_in = {strlen(lon_str), lon_str};

  etdb_status_t status = etdb_string("+ERROR");
  etdb_str_t    spset_cmd= etdb_string("spset");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &spset_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, id);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &lat_in);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &lon_in);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);

  etdb_bytes_t* resp = etdb_client_request(client);
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status           = bv->str;
  }
  return status;  
}

etdb_status_t
etdb_spatial_point_get(etdb_client_t *client, etdb_str_t *id, 
                double *lat, double *lon, char *code, int *code_len)
{
  etdb_status_t status = etdb_string("+ERROR");
  etdb_str_t    spget_cmd= etdb_string("spget");
  etdb_str_t    value;

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &spget_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, id);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);

  etdb_bytes_t* resp = etdb_client_request(client);
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status           = bv->str;
    if(bv->queue.next != &(resp->queue)){
      bv     = (etdb_bytes_t*)(bv->queue.next);
      value  = bv->str;
      size_t     split_num = 3;
      etdb_str_t splits[3];
      etdb_str_split(&value, ' ', splits, &split_num);
      if(split_num == 3){
        etdb_atof(splits[0].data, splits[0].len, lat);
        etdb_atof(splits[1].data, splits[1].len, lon);
        memcpy(code, splits[2].data + 1, splits[2].len - 2);
        *code_len = splits[2].len - 2;
      }
    }
  }
  return status; 
}

etdb_status_t
etdb_spatial_point_rect_query(etdb_client_t *client, 
              double lat1, double lat2, double lon1, double lon2, etdb_bytes_t **value_list)
{
  char lat1_str[128], lat2_str[128], lon1_str[128], lon2_str[128];
  sprintf(lat1_str, "%f", lat1);
  sprintf(lat2_str, "%f", lat2);
  sprintf(lon1_str, "%f", lon1);
  sprintf(lon2_str, "%f", lon2);

  etdb_str_t lat1_in = {strlen(lat1_str), lat1_str};
  etdb_str_t lon1_in = {strlen(lon1_str), lon1_str};
  etdb_str_t lat2_in = {strlen(lat2_str), lat2_str};
  etdb_str_t lon2_in = {strlen(lon2_str), lon2_str};

  etdb_status_t status = etdb_string("+ERROR");
  etdb_str_t    sprect_cmd= etdb_string("sprect");

  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &sprect_cmd);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &lat1_in);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &lat2_in);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &lon1_in);
  etdb_buf_append_record(etdb_cli_conn(client)->buf_out, &lon2_in);
  etdb_buf_append_record_tail(etdb_cli_conn(client)->buf_out);

  etdb_bytes_t* resp = etdb_client_request(client);
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status           = bv->str;
    *value_list      = resp;
  }
  return status;    
}
