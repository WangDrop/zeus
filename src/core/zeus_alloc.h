/*
 *  email : ydzhao1992#gmail.com
 *   date : 2014-12-12
 */

// This is the implemention of the memory pool.(multi-process & single thread & destroy finally)
// It can be used with object pool.

#ifndef __ZEUS_ALLOC_H_H_H__
#define __ZEUS_ALLOC_H_H_H__

#include "zeus_common.h"

#define ZEUS_ALIGN (sizeof(zeus_ulong_t))
#define ZEUS_MEMORY_POOL_ALIGN (0x10)
#define ZEUS_MEMORY_POOL_BLOCK_MAXSIZE (0x1000)
#define ZEUS_MEMORY_POOL_BLOCK_ITER_MAXTIME (0x3)

#define zeus_align(a,b) (zeus_uchar_t *)(((zeus_ulong_t)(a) + ((zeus_ulong_t)(b) - 0x1)) & (~((zeus_ulong_t)(b) - 0x1)))
#define zeus_addr_greater_or_equal(a,b) (zeus_uint_t)((zeus_ulong_t)(a) >= (zeus_ulong_t)(b))
#define zeus_addr_delta(a,b) (zeus_size_t)((zeus_uchar_t *)(a) - (zeus_uchar_t *)(b))
#define zeus_addr_add(a,b) (void *)((zeus_uchar_t *)(a) + (b))

typedef struct zeus_memory_pool_block_s zeus_memory_pool_block_t;
typedef struct zeus_memory_pool_large_s zeus_memory_pool_large_t;
typedef struct zeus_memory_pool_data_s zeus_memory_pool_data_t;

struct zeus_memory_pool_data_s{

    void *start;
    void *current;
    void *end;

};

struct zeus_memory_pool_s{
    zeus_memory_pool_block_t *next_block;
    zeus_memory_pool_large_t *next_large_block;
    zeus_memory_pool_block_t *iter_block;
    zeus_log_t *log;
};

struct zeus_memory_pool_block_s{
    zeus_memory_pool_data_t data;
    zeus_memory_pool_block_t *next_block;
};

struct zeus_memory_pool_large_s{
    zeus_memory_pool_data_t data;
    zeus_memory_pool_large_t *next_large_block;
};



zeus_memory_pool_t *zeus_create_memory_pool(zeus_log_t *log);
void *zeus_alloc_p(zeus_size_t size);
void zeus_memory_pool_destroy(zeus_memory_pool_t *pool);

void *zeus_memory_alloc(zeus_memory_pool_t *pool,zeus_size_t size);
zeus_memory_pool_block_t *zeus_memory_alloc_block(void);
zeus_memory_pool_large_t *zeus_memory_alloc_large(zeus_size_t size);

#endif
