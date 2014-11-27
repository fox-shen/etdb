
#include <etdb.h>

extern etdb_module_t etdb_kv_module;
extern etdb_module_t etdb_hash_module;
extern etdb_module_t etdb_list_module;
extern etdb_module_t etdb_set_module;
extern etdb_module_t etdb_sidx_module;
extern etdb_module_t etdb_sys_module;

etdb_module_t* etdb_modules[]={
  &etdb_kv_module,
  &etdb_hash_module,
  &etdb_list_module,
  &etdb_set_module,
  &etdb_sidx_module,
  &etdb_sys_module,
  NULL
};

