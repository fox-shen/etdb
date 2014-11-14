#ifndef H_EMDB_STRING_H
#define H_EMDB_STRING_H

typedef struct{
  size_t  len;
  uint8_t *data;
}emdb_str_t;

#define emdb_string(str) {sizeof(str) - 1, (uint8_t*)str};
#define emdb_null_string {0, NULL}

size_t emdb_str_split(emdb_str_t *s, uint8_t split, emdb_str_t *splits, size_t *split_num);

#endif
