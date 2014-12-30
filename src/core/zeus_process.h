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

};


zeus_process_t *zeus_create_process(void);
zeus_status_t zeus_init_process(zeus_process_t *);


#endif
