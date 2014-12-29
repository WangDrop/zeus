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

typedef struct zeus_hash_data_s zeus_hash_data_t;

struct zeus_hash_data_s{

    zeus_string_t name;

    zeus_string_t value;

    zeus_hash_data_t *next;

};

struct zeus_hash_s{
    
    zeus_hash_data_t *bucket;

    zeus_size_t bucket_size;

};

zeus_hash_t *zeus_create_hash_table(zeus_memory_pool_t *pool);

#endif
