#include <etdb.h>

static int etdb_hash_init_handler(void *args);
static int etdb_hash_set_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
static int etdb_hash_get_handler(etdb_bytes_t *args, etdb_connection_t *conn, etdb_bytes_t *resp);
#define ETDB_HASH_HEAD '0'


