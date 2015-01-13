/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-28
 */

// The process structure is crucial.

#ifndef __ZEUS_PROCESS_H_H_H__
#define __ZEUS_PROCESS_H_H_H__

#include "zeus_common.h"

struct zeus_process_s{
    
    zeus_memory_pool_t *pool; //memory pool

    zeus_log_t *log; //log

    zeus_config_t *config; //config

    zeus_list_data_t *buffer_pool; //object pool

	zeus_log_t *old; // old log

	
	
	zeus_uid_t uid; //user id

	zeus_gid_t gid; //group id

	zeus_uint_t resolution; //time resolution

	zeus_port_t port; //port

	zeus_uint_t worker; //worker process count
	
	zeus_string_t *pid_run_flag_path; //pid file path

	
};


zeus_process_t *zeus_create_process(void);
zeus_status_t zeus_init_process(zeus_process_t *);


#endif
