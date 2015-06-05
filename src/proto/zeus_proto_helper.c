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

void zeus_proto_helper_get_opcode_and_pktlen(zeus_event_t *ev,zeus_uchar_t *opcode,zeus_uint_t *len){
    
    zeus_size_t idx;
    zeus_size_t leftsize;
    zeus_size_t len_current;
    zeus_buffer_t *buf = (zeus_buffer_t *)ev->buffer->head->d;

    *opcode = *(zeus_uchar_t *)(buf->current);
    leftsize = zeus_addr_delta(buf->last,buf->current);
    if(leftsize > ZEUS_PROTO_OPCODE_SIZE + ZEUS_PROTO_DATA_LEN_SIZE){
        *len = *(zeus_uint_t *)zeus_addr_add(buf->current,ZEUS_PROTO_OPCODE_SIZE);
        *len = (zeus_uint_t)ntohl(*len);
    }else{
        len_current = leftsize - ZEUS_PROTO_OPCODE_SIZE;
        for(idx = 0 ; idx < len_current ; ++ idx){
            *(zeus_uchar_t *)zeus_addr_add(len,idx) = *(zeus_uchar_t *)zeus_addr_add(buf->current,ZEUS_PROTO_OPCODE_SIZE + idx);
        }
        buf = (zeus_buffer_t *)ev->buffer->head->next->d;
        for(idx = 0 ; idx < ZEUS_PROTO_DATA_LEN_SIZE ; ++ idx){
            *(zeus_uchar_t *)zeus_addr_add(len,(len_current + idx)) = *(zeus_uchar_t *)zeus_addr_add(buf->current,idx);
        }
    }
    
    return ;

}
