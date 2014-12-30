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
void zeus_log_config(zeus_config_t *config,zeus_log_t *log);

#endif
