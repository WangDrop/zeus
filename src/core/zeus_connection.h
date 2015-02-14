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

    zeus_sockaddr_in_t *peer;

    zeus_socklen_t *peerlen;

};


zeus_list_data_t *zeus_create_connection_list_node(zeus_process_t *);

zeus_status_t zeus_recycle_connection_list_node_to_pool(zeus_process_t *,zeus_list_data_t *);


#endif
