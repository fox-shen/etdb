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
etdb_client_request(etdb_client_t *client, uint8_t *cmd, size_t cmd_len)
{
  etdb_connect_send_cmd(client->link, cmd, cmd_len);
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

etdb_status_t 
etdb_set(etdb_client_t *client, etdb_str_t *key, etdb_str_t *value)
{
  etdb_status_t status = etdb_string("+FAIL");
  int len = key->len + value->len + sizeof("set") - 1 + 2;
  uint8_t *pos;

  if(len > client->command_buf_len){
    client->command_buf_len *= 2;
    client->command_buf      = (uint8_t*)etdb_realloc(client->command_buf, client->command_buf_len);
  }

  pos = memcpyn(client->command_buf, "set ", sizeof("set ") - 1);
  pos = memcpyn(pos, key->data, key->len); 
  pos = memcpyn(pos, " ", sizeof(" ") - 1);
  pos = memcpyn(pos, value->data, value->len);
   
  etdb_bytes_t* resp = etdb_client_request(client, client->command_buf, len); 
  if(resp != NULL && !etdb_queue_empty(&(resp->queue))){
    etdb_bytes_t *bv = (etdb_bytes_t*)(resp->queue.next);
    status           = bv->str;
  }
  return status; 
}

etdb_status_t 
etdb_get(etdb_client_t *client, etdb_str_t *key, etdb_str_t *value)
{
  etdb_status_t status = etdb_string("+FAIL");
  int len = key->len + sizeof("get") - 1 + 1;
  uint8_t *pos;

  if(len > client->command_buf_len){
    client->command_buf_len *= 2;
    client->command_buf      = (char*)etdb_realloc(client->command_buf, client->command_buf_len);
  }

  pos = memcpyn(client->command_buf, "get ", sizeof("get ") - 1);
  pos = memcpyn(pos, key->data, key->len);

  etdb_bytes_t* resp = etdb_client_request(client, client->command_buf, len); 
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
