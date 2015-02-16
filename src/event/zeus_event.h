/*
 *	Email : ydzhao1992#gmail.com
 *	 Date : 2015-01-15
 */

// This is the interface of event.


#ifndef __ZEUS_EVENT_H_H_H__
#define __ZEUS_EVENT_H_H_H__

#include "../core/zeus_common.h"

typedef zeus_status_t (zeus_event_handler)(zeus_process_t *,zeus_event_t *);
typedef int zeus_event_rw_flag_t;

#define ZEUS_EVENT_ON 0x1
#define ZEUS_EVENT_OFF 0x0
#define ZEUS_EVENT_CNT 0xa

struct zeus_event_s{

    zeus_connection_t *connection;

    zeus_list_t *buffer;

    zeus_event_handler *handler;

    zeus_event_timer_rbnode_t *timeout_rbnode;

    zeus_event_handler *timeout_handler;
    
    zeus_event_status_t timeout;

};

zeus_event_t *zeus_create_event(zeus_process_t *);
zeus_status_t zeus_master_event_loop(zeus_process_t *);
zeus_status_t zeus_event_loop(zeus_process_t *);

zeus_status_t zeus_event_loop_init_signal(zeus_process_t *);
zeus_status_t zeus_event_loop_init_connection(zeus_process_t *);

zeus_status_t zeus_event_create_epfd(zeus_process_t *);
zeus_status_t zeus_event_init_epoll(zeus_process_t *);

#endif
