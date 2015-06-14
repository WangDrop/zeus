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

    if(zeus_proto_buffer_write_uint(p,ev,ZEUS_PROTO_CHILDREN_PROCESS_CNT_SIZE) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"generate ack ok packet (message size) error");
        return ZEUS_ERROR;
    }

    if(zeus_proto_buffer_write_uint(p,ev,p->worker) == ZEUS_ERROR){
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

zeus_status_t zeus_proto_helper_generate_reset_load_balance_packet(zeus_process_t *p,zeus_event_t *ev){

    if(zeus_proto_buffer_write_byte(p,ev,ZEUS_PROTO_RESET_LOAD_BALANCE_INS) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"generate reset load balance packet (message type) error");
        return ZEUS_ERROR;
    }

    if(zeus_proto_buffer_write_uint(p,ev,ZEUS_PROTO_IDX_SIZE) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"generate reset load balance packet (message size) error");
        return ZEUS_ERROR;
    }

    if(zeus_proto_buffer_write_uint(p,ev,p->pidx - 1) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"generate reset load balance packet (message) error");
        return ZEUS_ERROR;
    }

    return ZEUS_OK;

}

void zeus_proto_helper_get_opcode_and_pktlen(zeus_event_t *ev,zeus_uchar_t *opcode,zeus_uint_t *len){
    
    zeus_size_t leftsize;
    zeus_list_data_t *node = ev->buffer->head;
    zeus_buffer_t *buf = (zeus_buffer_t *)ev->buffer->head->d;
    zeus_char_t *c;

    zeus_size_t sz = sizeof(zeus_uint_t);
    
    leftsize = zeus_addr_delta(buf->last,buf->current);

    if(leftsize >= sizeof(zeus_char_t)){
        *opcode = *(zeus_uchar_t *)(buf->current);
    }else{
        node = node->next;
        buf = (zeus_buffer_t *)node->d;
        *opcode = *(zeus_uchar_t *)(buf->current);
    }
    c = (zeus_char_t *)zeus_addr_add(buf->current,sizeof(zeus_char_t));
    
    leftsize = zeus_addr_delta(buf->last,c);

    if(leftsize > ZEUS_PROTO_DATA_LEN_SIZE){
        *len = *(zeus_uint_t *)c;
    }else{
        while(sz > 0){
            leftsize = zeus_addr_delta(buf->last,c);
            if(leftsize >= sz){
                zeus_memcpy((zeus_char_t *)zeus_addr_add(len,sizeof(zeus_uint_t) - sz),c,sz);
                c = (zeus_char_t *)zeus_addr_add(c,sz);
                sz = 0;
            }else{
                zeus_memcpy((zeus_char_t *)zeus_addr_add(len,sizeof(zeus_uint_t) - sz),c,leftsize);
                sz -= leftsize;
                node = node->next;
                buf = (zeus_buffer_t *)node->d;
                c = (zeus_char_t *)buf->current;
            }
        }
    }
    *len = (zeus_uint_t)ntohl(*len);
    
    return ;

}

zeus_status_t zeus_proto_helper_check_hash(zeus_string_t *s,zeus_char_t *cbuf,zeus_size_t sz){
    
    if(zeus_string_and_carr_equal(s,cbuf,sz) == 0){
        return ZEUS_OK;
    }else{
        return ZEUS_ERROR;
    }

}

void zeus_proto_helper_move_forward_opcode_and_pklen(zeus_process_t *p,zeus_event_t *ev){
    
    zeus_size_t leftsz;
    zeus_size_t opcode_and_len = ZEUS_PROTO_OPCODE_SIZE + ZEUS_PROTO_DATA_LEN_SIZE;
    zeus_list_data_t *q;
    zeus_buffer_t *buf;

    while(opcode_and_len > 0){

        q = ev->buffer->head;
        buf = (zeus_buffer_t *)q->d;
        leftsz = zeus_addr_delta(buf->last,buf->current);
        if(leftsz >= opcode_and_len){
            buf->current = zeus_addr_add(buf->current,opcode_and_len);
            ev->buflen -= opcode_and_len;
            opcode_and_len = 0;
            if(buf->current == buf->last){
                zeus_delete_list(ev->buffer,q);
                zeus_recycle_buffer_list_node_to_pool(p,q);
            }
        }else{
            buf->current = zeus_addr_add(buf->current,leftsz);
            ev->buflen -= leftsz;
            opcode_and_len -= leftsz;
            zeus_delete_list(ev->buffer,q);
            zeus_recycle_buffer_list_node_to_pool(p,q);
        }
    }

    return ;

}

void zeus_proto_helper_cp_data_from_buf_to_carr(zeus_process_t *p,zeus_event_t *ev,zeus_char_t *cbuf,zeus_size_t sz){

    zeus_size_t leftsz;
    zeus_buffer_t *buf;
    zeus_list_data_t *q;
    zeus_char_t *r = cbuf;
    
    zeus_proto_helper_move_forward_opcode_and_pklen(p,ev);
    
    while(sz > 0){

        q = ev->buffer->head;
        buf = (zeus_buffer_t *)(q->d);
        leftsz = zeus_addr_delta(buf->last,buf->current);

        if(leftsz >= sz){

            zeus_memcpy((void *)r,buf->current,sz);
            buf->current = zeus_addr_add(buf->current,sz);
            ev->buflen -= sz;
            sz = 0;
            if(buf->current == buf->last){
                zeus_delete_list(ev->buffer,q);
                zeus_recycle_buffer_list_node_to_pool(p,q);
            }

        }else{

            zeus_memcpy((void *)r,buf->current,leftsz);
            r = (zeus_char_t *)zeus_addr_add(r,leftsz);
            sz -= leftsz;
            ev->buflen -= leftsz;
            zeus_delete_list(ev->buffer,q);
            zeus_recycle_buffer_list_node_to_pool(p,q);

        }

    }

    return ;

}

zeus_status_t zeus_proto_helper_get_channel_index(zeus_process_t *p,zeus_event_t *ev,zeus_idx_t *idx){

    
    zeus_proto_helper_move_forward_opcode_and_pklen(p,ev);

    return zeus_proto_buffer_read_uint(p,ev,(zeus_uint_t *)idx);


}

zeus_status_t zeus_proto_helper_set_connection_privilege(zeus_connection_t *conn,zeus_uint_t ins){

    zeus_uint_t privilege_idx = ins / sizeof(zeus_uint_t);
    zeus_uint_t privilege_loc = ins % sizeof(zeus_uint_t);
    
    if(ins >= ZEUS_PROTO_INS_MAX){
        return ZEUS_ERROR;
    } 

    conn->privilege[privilege_idx] |= (1 << privilege_loc);

    return ZEUS_OK;

}

zeus_status_t zeus_proto_helper_check_opcode_privilege_of_connection(zeus_connection_t *conn,zeus_uchar_t opcode){
    
    zeus_uint_t ins = (zeus_uint_t)opcode;
    zeus_uint_t privilege_idx = ins / sizeof(zeus_uint_t);
    zeus_uint_t privilege_loc = ins % sizeof(zeus_uint_t);

    if(ins >= ZEUS_PROTO_INS_MAX){
        return ZEUS_ERROR;
    }

    if(conn->privilege[privilege_idx] & (1 << privilege_loc)){
        return ZEUS_OK;
    }else{
        return ZEUS_ERROR;
    }


}
