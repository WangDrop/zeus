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

zeus_status_t zeus_proto_helper_generate_trans_socket_ack_packet(zeus_process_t *p,zeus_event_t *ev){

    if(zeus_proto_buffer_write_byte(p,ev,ZEUS_PROTO_TRANS_SOCKET_ACK_INS) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"generate trans socket ack packet (message type) error");
        return ZEUS_ERROR;
    }

    if(zeus_proto_buffer_write_uint(p,ev,ZEUS_PROTO_IDX_SIZE) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"generate trans socket ack packet (message size) error");
        return ZEUS_ERROR;
    }

    if(zeus_proto_buffer_write_uint(p,ev,p->pidx - 1) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"generate trans socket ack packet (message) error");
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

zeus_status_t zeus_proto_helper_check_hash(zeus_string_t *s,zeus_char_t *cbuf,zeus_size_t sz){
    
    if(zeus_string_and_carr_equal(s,cbuf,sz) == 0){
        return ZEUS_OK;
    }else{
        return ZEUS_ERROR;
    }

}

void zeus_proto_helper_cp_data_from_buf_to_carr(zeus_process_t *p,zeus_event_t *ev,zeus_char_t *cbuf,zeus_size_t sz){

    zeus_size_t leftsz;
    zeus_buffer_t *buf;
    zeus_list_data_t *q;
    zeus_char_t *r = cbuf;

    zeus_uint_t ignore_opcode_and_len = 1;
    zeus_size_t opcode_add_len = ZEUS_PROTO_OPCODE_SIZE + ZEUS_PROTO_DATA_LEN_SIZE;

    ev->buflen -= opcode_add_len;
    
    while(sz > 0){

        q = ev->buffer->head;
        buf = (zeus_buffer_t *)(q->d);

        if(ignore_opcode_and_len){
            while(opcode_add_len > 0){
                leftsz = zeus_addr_delta(buf->last,buf->current);
                if(leftsz >= opcode_add_len){
                    buf->current = zeus_addr_add(buf->current,opcode_add_len);
                    opcode_add_len = 0;
                    if(buf->current == buf->last){
                        if(q->next){
                            q->next->prev = NULL;
                        }
                        ev->buffer->head = q->next;
                        if(!ev->buffer->head){
                            ev->buffer->tail = NULL;
                        }
                        zeus_recycle_buffer_list_node_to_pool(p,q);
                        q = ev->buffer->head;
                        buf = (zeus_buffer_t *)(q->d);
                    }
                }else{
                    buf->current = zeus_addr_add(buf->current,leftsz);
                    opcode_add_len -= leftsz;
                    if(q->next){
                        q->next->prev = NULL;
                    }
                    ev->buffer->head = q->next;
                    if(!ev->buffer->head){
                        ev->buffer->tail = NULL;
                    }
                    zeus_recycle_buffer_list_node_to_pool(p,q);
                    q = ev->buffer->head;
                    buf = (zeus_buffer_t *)(q->d);
                }
            }
            ignore_opcode_and_len = 0;
        }

        leftsz = zeus_addr_delta(buf->last,buf->current);

        if(leftsz >= sz){

            zeus_memcpy((void *)r,buf->current,sz);
            buf->current = zeus_addr_add(buf->current,sz);
            ev->buflen -= sz;
            sz = 0;
            if(buf->current == buf->last){
                if(q->next){
                    q->next->prev = NULL;
                }
                ev->buffer->head = q->next;
                if(!ev->buffer->head){
                    ev->buffer->tail = NULL;
                }
                zeus_recycle_buffer_list_node_to_pool(p,q);
            }

        }else{

            zeus_memcpy((void *)r,buf->current,leftsz);
            r = (zeus_char_t *)zeus_addr_add(r,leftsz);
            sz -= leftsz;
            ev->buflen -= sz;
            if(q->next){
                q->next->prev = NULL;
            }
            ev->buffer->head = q->next;
            if(!ev->buffer->head){
                ev->buffer->tail = NULL;
            }
            zeus_recycle_buffer_list_node_to_pool(p,q);

        }

    }

    return ;

}
