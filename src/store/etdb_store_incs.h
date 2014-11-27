#ifndef ETDB_STORE_INCS_H
#define ETDB_STORE_INCS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stddef.h>

#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <signal.h>

#define ETDB_PAGE_SIZE 8192

#define ETDB_STORE_INDEX_NO_PATH       -1
#define ETDB_STORE_INDEX_NO_VALUE      -2
#define ETDB_STORE_INDEX_MAX_TRIAL     1

typedef int64_t etdb_store_id_t;
#define ETDB_STORE_VALUE_LIMIT         0xefffffffffffffff

#include <etdb_version.h>
#include <etdb_alloc.h>
#include <etdb_palloc.h>
#include <etdb_string.h>
#include <etdb_log.h>
#include <etdb_queue.h>
#include <etdb_stack.h>
#include <etdb_bytes.h>
#include <etdb_trie.h>
#include <etdb_lru.h>
#include <etdb_store.h>

#endif
