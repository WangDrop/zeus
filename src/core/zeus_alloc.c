/*
 *  email : ydzhao1992#gmail.com
 *   date : 2014-12-12
 */

#include "zeus_alloc.h"

// create the memory pool
zeus_memory_pool_t *zeus_create_memory_pool(zeus_log_t *log){
    
    zeus_memory_pool_t *alloc_pool;
    zeus_memory_pool_block_t *alloc_block;

    alloc_pool = (zeus_memory_pool_t *)zeus_alloc_p(sizeof(zeus_memory_pool_t));

    if(alloc_pool == NULL){
        zeus_write_log(log,ZEUS_LOG_ERROR,"%s","memory pool create error");
        return NULL;
    }
    
    alloc_block = zeus_memory_alloc_block();

    if(alloc_block == NULL){
        zeus_write_log(log,ZEUS_LOG_ERROR,"%s","alloc first block error");
        free(alloc_pool);
        return NULL;
    }


    alloc_pool->next_large_block = NULL;
    alloc_pool->next_block = alloc_block;
    alloc_pool->iter_block = alloc_block;
    alloc_pool->log = log;

    return alloc_pool;

}

void *zeus_memory_alloc(zeus_memory_pool_t *pool,zeus_size_t size){
    
    zeus_uint_t idx;
    zeus_memory_pool_block_t *iter;
    zeus_memory_pool_block_t *alloc;
    zeus_memory_pool_large_t *alloc_large;
    zeus_uchar_t *p;

    if(size > ZEUS_MEMORY_POOL_BLOCK_MAXSIZE){
        // go to alloc large block
        alloc_large = zeus_memory_alloc_large(size);
        if(alloc_large == NULL){
            zeus_write_log(pool->log,ZEUS_LOG_ERROR,"%s","alloc large block error");
            return NULL;
        }
        alloc_large->next_large_block = pool->next_large_block;
        pool->next_large_block = alloc_large;
        return (void *)alloc_large->data.start;

    }else{

        idx = 0;
        iter = pool->iter_block;

        while(idx < ZEUS_MEMORY_POOL_BLOCK_ITER_MAXTIME && iter){
            p = zeus_align(iter->data.current,ZEUS_ALIGN);
            if(zeus_addr_greater_or_equal(iter->data.end,p) && zeus_addr_delta(iter->data.end,p) >= size){
                // find the block
                iter->data.current = zeus_addr_add(p,size);
                pool->iter_block = iter;
                return (void *)p;
            }else{
                ++ idx;
                iter = iter->next_block;
            }
        }
        
        // go to alloc block
        alloc = zeus_memory_alloc_block();
        if(alloc == NULL){
            zeus_write_log(pool->log,ZEUS_LOG_ERROR,"%s","alloc memory block error");
            if(iter == NULL){
                iter = pool->iter_block;
            }
            return NULL;
        }
        alloc->next_block = pool->next_block;
        pool->next_block = alloc;

        p = (zeus_uchar_t *)alloc->data.current;
        alloc->data.current = zeus_addr_add(p,size);

        if(iter == NULL){
            iter = pool->iter_block;
        }

        return (void *)p;

    }
    
    return NULL;
}

zeus_memory_pool_block_t *zeus_memory_alloc_block(void){

    zeus_memory_pool_block_t *alloc_block;
    void *alloc_data;

    alloc_block = (zeus_memory_pool_block_t *)zeus_alloc_p(sizeof(zeus_memory_pool_block_t));

    if(alloc_block == NULL){
        return NULL;
    }

    alloc_data = zeus_alloc_p(ZEUS_MEMORY_POOL_BLOCK_MAXSIZE);
    
    if(alloc_data == NULL){
        free(alloc_block);
        return NULL;
    }

    alloc_block->data.start = alloc_data;
    alloc_block->data.current = alloc_data;
    alloc_block->data.end = zeus_addr_add(alloc_data,ZEUS_MEMORY_POOL_BLOCK_MAXSIZE);
    alloc_block->next_block = NULL;

    return alloc_block;

}

zeus_memory_pool_large_t *zeus_memory_alloc_large(zeus_size_t size){
    
    zeus_memory_pool_large_t *alloc_large;
    void *alloc_data;

    alloc_large = (zeus_memory_pool_large_t *)zeus_alloc_p(sizeof(zeus_memory_pool_large_t));

    if(alloc_large == NULL){
        return NULL;
    }

    alloc_data = zeus_alloc_p(size);

    if(alloc_data == NULL){
        free(alloc_large);
        return NULL;
    }
    
    alloc_large->data.start = alloc_data;
    alloc_large->data.current = zeus_addr_add(alloc_data,size);
    alloc_large->data.end = zeus_addr_add(alloc_data,size);
    alloc_large->next_large_block = NULL;
    
    return alloc_large;

}

void *zeus_alloc_p(zeus_size_t size){
    
    int err;
    void *alloc;

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
    err = posix_memalign(&alloc,ZEUS_MEMORY_POOL_ALIGN,size);
    if(err == 0){
        //
    }else{
        alloc = NULL;
        //
    }
#elif (_ISOC11_SOURCE)
    alloc = aligned_alloc(ZEUS_MEMORY_POOL_ALIGN,size);
#else
    alloc = malloc(size);
#endif

    return alloc;

}

void zeus_memory_pool_destroy(zeus_memory_pool_t *pool){
    
    zeus_memory_pool_large_t *pl,*ql;
    zeus_memory_pool_block_t *pb,*qb;
    
    pl = pool->next_large_block;

    while(pl){
        ql = pl;
        pl = pl->next_large_block;
        free(ql->data.start);
        free(ql);
    }

    pb = pool->next_block;

    while(pb){
        qb = pb;
        pb = pb->next_block;
        free(qb->data.start);
        free(qb);
    }

    free(pool);

    return ;

}
