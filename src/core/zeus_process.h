/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-28
 */

// The process structure is crucial.

#ifndef __ZEUS_PROCESS_H_H_H__
#define __ZEUS_PROCESS_H_H_H__


#include "zeus_common.h"


#define ZEUS_MASTER_PROCESS_INDEX -1

#define ZEUS_GATEWAY_PROCESS_INDEX 0

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

    zeus_fd_t pid_run_flag_fd; //pid file fd

    zeus_uint_t max_connection; // max connection



    zeus_pid_t pid; // process id;

    zeus_idx_t pidx; // process idx , -1 means master , 0 means gateway

    zeus_fd_t listenfd; // listen fd

    zeus_int_t **channel; // channel to connection gateway/worker process ,0 for gateway , 1- for worker
	
    volatile zeus_pid_t *child;	// record gateway/worker pids


    zeus_size_t arg_sz;
	
    zeus_char_t **arg;  

    zeus_int_t record_argc;

    zeus_char_t **record_argv;

    zeus_int_t record_envc;

    zeus_char_t **record_environ;

};

extern volatile zeus_atomic_t zeus_quit;
extern volatile zeus_atomic_t zeus_refork;
extern volatile zeus_atomic_t zeus_segv;

zeus_process_t *zeus_create_process(void);

zeus_status_t zeus_init_process(zeus_process_t *);

zeus_status_t zeus_record_command_and_env(zeus_process_t *,zeus_int_t,zeus_char_t **);

zeus_status_t zeus_update_process_flag(zeus_process_t *);


#endif
