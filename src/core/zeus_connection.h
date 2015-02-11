/*
 *	Email : ydzhao1992#gmail.com
 *	 Date : 2015-01-15
 */


// This is the interface of the connections , tcp or unix sockets.


#ifndef __ZEUS_CONNECTION_H_H_H__
#define __ZEUS_CONNECTION_H_H_H__

#include "zeus_common.h"

struct zeus_connection_s{

	zeus_fd_t fd;

    zeus_event_t *rd;

    zeus_event_t *wr;

    zeus_event_status_t rdstatus;

    zeus_event_status_t wrstatus;

};


zeus_connection_t *zeus_create_connection_node(zeus_process_t *);


#endif
