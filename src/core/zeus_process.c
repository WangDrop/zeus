/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-28
 */

// This is the implemention of the zeus process , it is crucial.

#include "zeus_process.h"

static zeus_char_t zeus_process_master_flag[] = "zeus:master";
static zeus_char_t zeus_process_gateway_flag[] = "zeus:gateway";
static zeus_char_t zeus_process_worker_flag[] = "zeus:worker";

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
    alloc_process->config = NULL;
    alloc_process->buffer_pool = NULL;
    alloc_process->connection_pool = NULL;
    alloc_process->old = NULL;
    alloc_process->pid_run_flag_path = NULL;

    alloc_process->pidx = -1;
    alloc_process->epfd = -1;
    alloc_process->timer = NULL;
    alloc_process->cache_time = NULL;

    alloc_process->record_envc = 0;
    alloc_process->record_argc = 0;
    alloc_process->record_argv = NULL;
    alloc_process->record_environ = NULL;

    alloc_process->arg = NULL;

    alloc_process->channel = NULL;
    alloc_process->child = NULL;
    alloc_process->save = NULL;
    alloc_process->worker_load = NULL;
    alloc_process->listen_connection_node = NULL;
    
    return alloc_process;

}

zeus_status_t zeus_init_process(zeus_process_t *process){
    
    if((process->config = zeus_create_config(process)) == NULL){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"init process config error");
        return ZEUS_ERROR;
    }

    if(zeus_init_config(process) == ZEUS_ERROR){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"init process config content error");
        return ZEUS_ERROR;
    }

    if(zeus_config_get_uid(process->config,process->log,&(process->uid)) == ZEUS_ERROR){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"get user id error");
        return ZEUS_ERROR;
    }

    if(zeus_config_get_gid(process->config,process->log,&(process->gid)) == ZEUS_ERROR){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"get group id error");
        return ZEUS_ERROR;
    }

    if(zeus_config_get_resolution(process->config,process->log,&(process->resolution)) == ZEUS_ERROR){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"get timer resolution error");
        return ZEUS_ERROR;
    }

	
    if(zeus_config_get_worker(process->config,process->log,&(process->worker)) == ZEUS_ERROR){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"get worker process number error");
        return ZEUS_ERROR;
    }

    if(zeus_config_get_max_connection(process->config,process->log,&(process->max_connection)) == ZEUS_ERROR){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"get max connection error");
        return ZEUS_ERROR;
    }

    if(zeus_config_get_port(process->config,process->log,&(process->port)) == ZEUS_ERROR){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"get port error");
        return ZEUS_ERROR;
    }
    
    if(zeus_config_get_manage_hash(process) == ZEUS_ERROR){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"get manage hash error");
        return ZEUS_ERROR;
    }

    if(zeus_config_get_log_path(process) == ZEUS_ERROR){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"get new log according config error");
        return ZEUS_ERROR;
    }

    if(zeus_config_get_pid_path(process) == ZEUS_ERROR){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"set pid flag file error");
        return ZEUS_ERROR;
    }

    if(zeus_update_process_flag(process) == ZEUS_ERROR){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"update master process flag error");
        return ZEUS_ERROR;
    }
	
    return ZEUS_OK;

}

zeus_status_t zeus_record_command_and_env(zeus_process_t *process,zeus_int_t argc,zeus_char_t **argv){
	
    zeus_int_t idx = 0;
    zeus_size_t sz = 0;
    zeus_size_t tmp = 0;
    zeus_uint_t env_cnt = 0;

    zeus_char_t *p;

    process->record_argc = argc;

    process->record_argv = (zeus_char_t **)zeus_memory_alloc(process->pool,sizeof(zeus_char_t *) * argc);

    if(process->record_argv == NULL){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"record argv error : when alloc array");
        return ZEUS_ERROR;
    }

    p = argv[0];

    for(idx = 0 ; idx != argc ; ++ idx){
		
        tmp = zeus_strlen(argv[idx]) + 1;

        process->record_argv[idx] = (zeus_char_t *)zeus_memory_alloc(process->pool,\
                                                   sizeof(zeus_char_t) * tmp);
        if(process->record_argv[idx] == NULL){
            zeus_write_log(process->log,ZEUS_LOG_ERROR,"record argv error : when alloc array element");
            return ZEUS_ERROR;
        }

        sz += tmp;
        p += tmp;

        zeus_memcpy(process->record_argv[idx],argv[idx],tmp);
	
    }

    if(p != environ[0]){
        zeus_write_log(process->log,ZEUS_LOG_NOTICE,"argv and environ are not continous : %x : %x",p,environ[0]);
        sz += zeus_addr_delta(environ[0],p);
        p = environ[0];
    }


    for(idx = 0 ; environ[idx] ; ++ idx){
        ++ process->record_envc;
    }

    process->record_environ = (zeus_char_t **)zeus_memory_alloc(process->pool, \
                                              sizeof(zeus_char_t) * process->record_envc);

    if(process->record_environ == NULL){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"record environment error : when alloc array");
        return ZEUS_ERROR;
    }

    for(idx = 0 ; environ[idx] ; ++ idx){
		
        tmp = zeus_strlen(environ[idx]) + 1;
        process->record_environ[idx] = (zeus_char_t *)zeus_memory_alloc(process->pool,sizeof(zeus_char_t) * tmp);

        if(process->record_environ[idx] == NULL){
            zeus_write_log(process->log,ZEUS_LOG_ERROR,"record environment error : when alloc array element");
            return ZEUS_ERROR;
        }

        p += tmp;

        sz += tmp;

        zeus_memcpy(process->record_environ[idx],environ[idx],tmp);

    }
	
    process->arg = argv;

    process->arg_sz = sz;
	
    return ZEUS_OK;

}

zeus_status_t zeus_update_process_flag(zeus_process_t *process){
	
    zeus_char_t *beg;
    zeus_char_t *end;
    zeus_int_t nwrite;

    if(process->record_argc > 1){
        process->arg[1] = NULL;
    }

    beg = process->arg[0];
    end = beg + process->arg_sz;
	
    switch(process->pidx){
        case -1:
            nwrite = snprintf(beg,zeus_addr_delta(end,beg),"%s",zeus_process_master_flag);
            break;
        case 0:
            nwrite = snprintf(beg,zeus_addr_delta(end,beg),"%s",zeus_process_gateway_flag);
            break;
        default:
            nwrite = snprintf(beg,zeus_addr_delta(end,beg),"%s",zeus_process_worker_flag);
            break;
    }
	
    beg = (beg + nwrite < end) ? (beg + nwrite) : end;

    if(beg < end){
        zeus_memset(beg,0,zeus_addr_delta(end,beg));
    }

    return ZEUS_OK;

}

