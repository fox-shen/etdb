#ifndef H_ETDB_STRING_H
#define H_ETDB_STRING_H

typedef struct{
  size_t  len;
  uint8_t *data;
}etdb_str_t;

#define etdb_string(str) {sizeof(str) - 1, (uint8_t*)str}
#define etdb_null_string {0, NULL}
#define memcpyn(dst, src, len) ((uint8_t*)memcpy(dst, src, len) + len)

#define etdb_swap(a, b) { a = a + b; b = a -b;  a = a - b; }

size_t etdb_str_split(etdb_str_t *s, uint8_t split, etdb_str_t *splits, size_t *split_num);
void   etdb_str_tolower(uint8_t* data, size_t len);
void etdb_fprintf(FILE *stream, const char *format, ...);

int etdb_atof(const uint8_t *data, size_t len, double *ret);
int etdb_atoi(const uint8_t *data, size_t len, int    *ret);

void etdb_trim(etdb_str_t *str);

#endif
