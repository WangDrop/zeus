/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-02-17
 */

// this is for buffer handler

#ifndef __ZEUS_PROTO_IO_H_H_H__
#define __ZEUS_PROTO_IO_H_H_H__

#include "../core/zeus_common.h"

zeus_status_t zeus_proto_buffer_write_byte(zeus_process_t *,zeus_event_t *,zeus_char_t);
zeus_status_t zeus_proto_buffer_write_uint(zeus_process_t *,zeus_event_t *,zeus_uint_t);
zeus_status_t zeus_proto_buffer_write_byte_array(zeus_process_t *,zeus_event_t *,zeus_char_t *,zeus_size_t);


#endif
