#ifndef H_ETDB_FILE_CONFIG_H
#define H_ETDB_FILE_CONFIG_H

extern int etdb_init_file_config(char *conf_file);
extern int etdb_destroy_file_config();

extern int etdb_file_config_get_int(const char *key, int ini);
extern const char* etdb_file_config_get_string(const char *key, const char *ini);

#endif
