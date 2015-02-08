/*
 *	Email : ydzhao1992#gmail.com
 *	 Date : 2015-01-15
 */

// This is the interface of event.


#ifndef __ZEUS_EVENT_H_H_H__
#define __ZEUS_EVENT_H_H_H__

#include "../core/zeus_common.h"

typedef zeus_status_t (zeus_event_handler)(zeus_event_t *);

#define ZEUS_EVENT_ON 1
#define ZEUS_EVENT_OFF 0

struct zeus_event_s{

    zeus_connection_t *connection;

	zeus_list_t *buffer;

    zeus_event_handler *handler;

    zeus_epoll_event_t *event;

};

zeus_event_t *zeus_create_event(zeus_process_t *);

zeus_status_t zeus_master_event_loop(zeus_process_t *);
zeus_status_t zeus_event_loop(zeus_process_t *);

zeus_status_t zeus_event_loop_init_signal(zeus_process_t *);
zeus_status_t zeus_event_loop_init_connection(zeus_process_t *);

zeus_status_t zeus_event_create_epfd(zeus_process_t *);

#endif
