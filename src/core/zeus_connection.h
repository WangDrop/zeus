/*
 *	Email : ydzhao1992#gmail.com
 *	 Date : 2015-01-15
 */


// This is the interface of the connections , tcp or unix sockets.


#ifndef __ZEUS_CONNECTION_H_H_H__
#define __ZEUS_CONNECTION_H_H_H__

#define ZEUS_IPV4_ADDRESS_STRING_SIZE 20

#include "zeus_common.h"

struct zeus_connection_s{

    zeus_fd_t fd;

    zeus_event_t *rd;

    zeus_event_t *wr;

    zeus_event_status_t rdstatus;

    zeus_event_status_t wrstatus;

    zeus_sockaddr_in_t *peer;

    zeus_socklen_t *peerlen;

    zeus_string_t *addr_string;

    zeus_uint_t quiting;

    zeus_list_data_t *node;
    
    zeus_uint_t *privilege;

    zeus_uint_t admin;

    zeus_uint_t check;

};


zeus_list_data_t *zeus_create_connection_list_node(zeus_process_t *);

zeus_connection_t *zeus_create_connection_array(zeus_process_t *,zeus_uint_t);

zeus_status_t zeus_recycle_connection_list_node_to_pool(zeus_process_t *,zeus_list_data_t *);


#endif
