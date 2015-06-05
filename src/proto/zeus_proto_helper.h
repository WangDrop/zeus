/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-02-16
 */

// some logic here．

#ifndef __ZEUS_PROTO_HELPER_H_H_H__
#define __ZEUS_PROTO_HELPER_H_H_H__

#include "../core/zeus_common.h"

#define ZEUS_ACK_OK_MESSAGE "zeus_ack_ok"
#define ZEUS_ACK_OK_MESSAGE_SIZE (sizeof(ZEUS_ACK_OK_MESSAGE))

zeus_status_t zeus_proto_helper_generate_ack_ok_packet(zeus_process_t *,zeus_event_t *);
void zeus_proto_helper_get_opcode_and_pktlen(zeus_event_t *,zeus_uchar_t *,zeus_uint_t *);

#endif
