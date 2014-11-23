#include <etdb.h>

size_t 
etdb_str_split(etdb_str_t *s, uint8_t split, etdb_str_t *splits, size_t *split_num)
{
  size_t i = 0, j = 0, pos = 0;
  for(; j < s->len; ++j){
    if(s->data[j] == split){
      if(pos < *split_num){
        etdb_str_t s_str = {j - i, s->data + i};
        splits[pos++]    = s_str;
        i                = j + 1;
      }else{
        return *split_num;
      }  
    }
  }
  if(pos < *split_num){
    etdb_str_t s_str = {j - i, s->data + i};
    splits[pos++]    = s_str;
    *split_num       = pos;
  }
  return *split_num;
}

void   
etdb_str_tolower(uint8_t* data, size_t len)
{
  size_t pos;
  for(pos = 0; pos < len; ++pos){
    if(data[pos] >= 'A' && data[pos] <= 'Z')
       data[pos] = 'a' + data[pos] - 'A';
  }
}

static void
etdb_vslprintf(FILE *stream, const char *format, va_list args)
{
  char buf[1024]; char* pos = buf;
  while(*format){
    if(*format == '%'){
      ++format; 
      switch(*format){
        case 'V':
          {
            etdb_str_t *v = va_arg(args, etdb_str_t*);
            pos = memcpy(buf, v->data, v->len);
            ++pos;
            *pos = '\0';
            fprintf(stream, "%s", buf);
          }
          break;
        case 'd':
         {
            
         }
      }
    }
    ++format;
  } 
}

void 
etdb_fprintf(FILE *stream, const char *format, ...)
{
  u_char *p;
  va_list args;

  va_start(args, format);
  etdb_vslprintf(stream, format, args);
  va_end(args);
}

int 
etdb_atof(const uint8_t *data, size_t len, double *ret)
{
  *ret       = 0;
  size_t i   = 0;
  int   flag = 1;
  float step = 1.0;

  while(i < len){
    if(i == 0){
      if(data[i] == '-')       flag = -1;
      else if(data[i] == '+')  flag = 1;
    }
    if(data[i] == '.')  step = 0.1;
    else if(data[i] >= '0' && data[i] <= '9'){
      if(step < 1){
        *ret += (data[i] - '0')*step;
        step = step * 0.1;
      }else{
        *ret = *ret*10 + (data[i] - '0');
      }
    }else{
      return -1;  
    } 
    ++i;
  }
  *ret = *ret * flag;
  return 0; 
}

int 
etdb_atoi(const uint8_t *data, size_t len, int *ret)
{
  *ret     = 0;
  int i    = 0;
  int flag = 1;
 
  while(i < len){
    if(i == 0){
      if(data[i] == '+')      flag = 1;
      else if(data[i] == '-') flag = -1;
    }
    if(data[i] >= '0' && data[i] <= '9'){
      *ret = *ret * 10 + (data[i] - '0');
    }else{
      return -1;
    }
    ++i;
  }
  *ret = *ret * flag;
  return 0;
}
