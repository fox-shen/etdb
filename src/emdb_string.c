#include <emdb.h>

size_t 
emdb_str_split(emdb_str_t *s, uint8_t split, emdb_str_t *splits, size_t *split_num)
{
  size_t i = 0, j = 0, pos = 0;
  for(; j < s->len; ++j){
    if(s->data[j] == split){
      if(pos < *split_num){
        emdb_str_t s_str = {j - i, s->data + i};
        splits[pos++]    = s_str;
        i                = j + 1;
      }else{
        return *split_num;
      }  
    }
  }
  if(pos < *split_num){
    emdb_str_t s_str = {j - i, s->data + i};
    splits[pos++]    = s_str;
    *split_num       = pos;
  }
  return *split_num;
}
