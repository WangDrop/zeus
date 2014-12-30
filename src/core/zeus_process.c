/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-28
 */

// This is the implemention of the zeus process , it is crucial.

#include "zeus_process.h"

zeus_process_t *zeus_create_process(void){
    
    zeus_log_t *alloc_log = zeus_create_log(NULL,ZEUS_LOG_NOTICE);
    if(alloc_log == NULL){
        zeus_write_log_to_stderr(ZEUS_LOG_ERROR,"create log error");
        return NULL;
    }

    zeus_memory_pool_t *alloc_pool = zeus_create_memory_pool(alloc_log);
    if(alloc_pool == NULL){
        zeus_write_log_to_stderr(ZEUS_LOG_ERROR,"create memory pool error");
        return NULL;
    }

    zeus_process_t *alloc_process = (zeus_process_t *)zeus_memory_alloc(alloc_pool,sizeof(zeus_process_t));
    if(alloc_process == NULL){
        zeus_write_log_to_stderr(ZEUS_LOG_ERROR,"create process structure error");
        return NULL;
    }

    alloc_process->pool = alloc_pool;
    alloc_process->log = alloc_log;

    return alloc_process;

}

zeus_status_t zeus_init_process(zeus_process_t *process){
    
    if((process->config = zeus_create_config(process->pool)) == NULL){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"init process config error");
        return ZEUS_ERROR;
    }

    if(zeus_init_config(process->config,process->pool) == ZEUS_ERROR){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"init process config content error");
        return ZEUS_ERROR;
    }
    
    return ZEUS_OK;

}
