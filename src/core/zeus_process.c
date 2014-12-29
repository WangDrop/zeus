/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-28
 */

// This is the implemention of the zeus process , it is crucial.

#include "zeus_process.h"

zeus_process_t *zeus_create_process(void){
    
    zeus_log_t *alloc_log = zeus_create_log(NULL,ZEUS_LOG_NOTICE);
    zeus_memory_pool_t *alloc_pool = zeus_create_memory_pool(alloc_log);

    zeus_process_t *alloc_process = (zeus_process_t *)zeus_memory_alloc(alloc_pool,sizeof(zeus_process_t));
    alloc_process->pool = alloc_pool;
    alloc_process->log = alloc_log;

    return alloc_process;

}
