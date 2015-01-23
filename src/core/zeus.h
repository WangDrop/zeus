/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-29
 */


#ifndef __ZEUS_H_H_H__
#define __ZEUS_H_H_H__

#include "zeus_common.h"


#define ZEUS_PID_MAX_SIZE 16

zeus_status_t zeus_check_already_run(zeus_process_t *);

zeus_status_t zeus_init_daemon(zeus_process_t *);

zeus_status_t zeus_daemon(zeus_process_t *);

zeus_status_t zeus_check_daemon_already_run(zeus_process_t *);

zeus_status_t zeus_prepare_spawn(zeus_process_t *);

zeus_status_t zeus_spawn(zeus_process_t *);

zeus_status_t zeus_master_prepare_loop(zeus_process_t *);

zeus_status_t zeus_prepare_loop(zeus_process_t *,zeus_idx_t);

zeus_status_t zeus_gateway_prepare_listen(zeus_process_t *);

zeus_status_t zeus_respawn(zeus_process_t *);

zeus_status_t zeus_inform_exit(zeus_process_t *);

zeus_status_t zeus_exit(zeus_process_t *);

#endif
