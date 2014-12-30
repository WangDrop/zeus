/*
 *  Email : ydzhao1992#gmail.com    
 *   Date : 2014-12-28
 */

// This is the interface of the hash table.
// Use this for configuration and others.


#ifndef __ZEUS_HASH_H_H_H__
#define __ZEUS_HASH_H_H_H__

#include "zeus_common.h"

#define ZEUS_HASH_BUCKET_SIZE 128
#define ZEUS_HASH_MAGIC_NUM 43

typedef struct zeus_hash_data_s zeus_hash_data_t;

struct zeus_hash_data_s{

    zeus_string_t *s;

    zeus_string_t *d;

    zeus_hash_data_t *next;

};

struct zeus_hash_s{
    
    zeus_hash_data_t **bucket;

    zeus_size_t bucket_size;

};

zeus_hash_t *zeus_create_hash_table(zeus_memory_pool_t *);
zeus_hash_data_t *zeus_get_hash_data_node(zeus_memory_pool_t *);
zeus_uint_t zeus_get_hash_key(zeus_string_t *);
void zeus_insert_hash_data_node(zeus_hash_t *,zeus_hash_data_t *);
void zeus_log_hash_table(zeus_hash_t *,zeus_log_t *log);

#endif
