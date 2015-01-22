/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-29
 */

// configuration reading

#ifndef __ZEUS_CONFIG_H_H_H__
#define __ZEUS_CONFIG_H_H_H__

#include "zeus_common.h"


struct zeus_config_s{

    zeus_hash_t *conf;

    zeus_string_t *path;

};

zeus_config_t *zeus_create_config(zeus_memory_pool_t *);
zeus_status_t zeus_init_config(zeus_config_t *,zeus_memory_pool_t *);
zeus_status_t zeus_parse_config_line(zeus_char_t *,zeus_int_t ,zeus_config_t *,zeus_memory_pool_t *);
void zeus_log_config(zeus_config_t *,zeus_log_t *);

zeus_status_t zeus_config_check(zeus_config_t *,zeus_log_t *,zeus_hash_data_t **,zeus_string_t *);

zeus_status_t zeus_config_get_uid(zeus_config_t *,zeus_log_t *,zeus_uid_t *);
zeus_status_t zeus_config_get_gid(zeus_config_t *,zeus_log_t *,zeus_gid_t *);
zeus_status_t zeus_config_get_resolution(zeus_config_t *,zeus_log_t *,zeus_uint_t *);
zeus_status_t zeus_config_get_worker(zeus_config_t *,zeus_log_t *,zeus_uint_t *);
zeus_status_t zeus_config_get_port(zeus_config_t *,zeus_log_t *,zeus_ushort_t *);
zeus_status_t zeus_config_get_max_connection(zeus_config_t *,zeus_log_t *,zeus_uint_t *);

zeus_status_t zeus_config_get_log_path(zeus_process_t *);
zeus_status_t zeus_config_get_pid_path(zeus_process_t *);


zeus_status_t zeus_config_gateworker_log_path(zeus_process_t *);

#endif
