/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-28
 */

// The process structure is crucial.

#ifndef __ZEUS_PROCESS_H_H_H__
#define __ZEUS_PROCESS_H_H_H__


#include "zeus_common.h"


#define ZEUS_MASTER_PROCESS_INDEX -1

#define ZEUS_DATA_GATEWAY_PROCESS_INDEX 0

struct zeus_process_s{
    
    zeus_memory_pool_t *pool;
    zeus_log_t *log;
    zeus_config_t *config;
    zeus_list_data_t *buffer_pool;
    zeus_list_data_t *connection_pool;
    zeus_log_t *old;
    zeus_log_t *save;

	
	
    zeus_uid_t uid;
    zeus_gid_t gid;
    zeus_uint_t resolution;
    zeus_port_t port;
    zeus_uint_t worker;
    zeus_string_t *pid_run_flag_path;
    zeus_fd_t pid_run_flag_fd;
    zeus_uint_t max_connection;
    zeus_string_t *manage_passwd;


    zeus_pid_t pid;
    /* -1 means master , 0 means gateway*/
    zeus_idx_t pidx;
    zeus_fd_t listenfd;
    zeus_int_t **channel;
    volatile zeus_pid_t *child;
    zeus_uint_t *worker_load;
    zeus_list_t **closing_connection;


    zeus_list_t *connection;
    zeus_fd_t epfd;
    zeus_event_timer_rbtree_t *timer;
    zeus_timeval_t *cache_time;


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
