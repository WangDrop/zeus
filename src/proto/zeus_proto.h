/*
 *  Email : ydzhao1992#gmail.com    
 *   Date : 2015-02-13
 */


// This is the interface of the application protocol


#ifndef __ZEUS_PROTO_H_H_H__
#define __ZEUS_PROTO_H_H_H__

#include "../core/zeus_common.h"

zeus_status_t zeus_proto_ack_connection(zeus_process_t *,zeus_event_t *);

zeus_status_t zeus_proto_solve_read_buf(zeus_process_t *,zeus_event_t *);

zeus_status_t zeus_proto_timeout_handler(zeus_process_t *,zeus_event_t *);

#endif
