/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-02-16
 */

#include "zeus_proto_helper.h"

zeus_status_t zeus_proto_helper_generate_ack_ok_packet(zeus_process_t *p,zeus_event_t *ev){

    if(zeus_proto_buffer_write_byte(p,ev,ZEUS_PROTO_ACK_CONNECTION_INS) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"generate ack ok packet (message type) error");
        return ZEUS_ERROR;
    }

    if(zeus_proto_buffer_write_uint(p,ev,ZEUS_ACK_OK_MESSAGE_SIZE) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"generate ack ok packet (message size) error");
        return ZEUS_ERROR;
    }

    if(zeus_proto_buffer_write_byte_array(p,ev,(zeus_char_t *)ZEUS_ACK_OK_MESSAGE,ZEUS_ACK_OK_MESSAGE_SIZE) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"generate ack ok packet (message) error");
        return ZEUS_ERROR;
    }

    return ZEUS_OK;

}
